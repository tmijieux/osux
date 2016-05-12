/*
 *  Copyright (©) 2015 Lucas Maugère, Thomas Mijieux
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "./python.h"

#ifndef _GNU_SOURCE
#	define _GNU_SOURCE
#endif // _GNU_SOURCE

#include <string.h>
#include <stdint.h>

#include "beatmap/beatmap.h"
#include "beatmap/timingpoint.h"
#include "beatmap/hitobject.h"
#include "beatmap/color.h"
#include "util/error.h"

#include "compiler.h"

#include "./pyfetch.h"
#include "../parser.h"

static osux_beatmap DEFAULT_MAP;

#define READ_VALUE(map, fieldName, pyObj, pyMethod, convMethod)	\
    do {                                                        \
        PyObject *pyTmp;                                        \
        pyTmp = PyDict_GetItemString(pyObj, #fieldName);        \
        if (pyTmp) {                                            \
            map->fieldName = convMethod(pyMethod(pyTmp));       \
            if (PyErr_Occurred()) {                             \
                puts("\n");                                     \
                PyErr_Print();                                  \
                puts("\n");                                     \
            }                                                   \
        }                                                       \
        else {                                                  \
            map->fieldName = DEFAULT_MAP.fieldName;             \
        }                                                       \
    } while (0)	       


#define PY_OBJ_TO_STR(X) PyString_AsString(PyObject_Str((X)))

#define READ_STRING(map, fieldName, pyObj)                      \
    READ_VALUE(map, fieldName, pyObj, PY_OBJ_TO_STR, strdup)    \
    
#define READ_DOUBLE(map, fieldName, pyObj)                      \
    READ_VALUE(map, fieldName, pyObj, PyFloat_AsDouble,)        \

#define READ_INT(map, fieldName, pyObj)                         \
    READ_VALUE(map, fieldName, pyObj, PyInt_AsLong, (uint32_t)) \



static void map_fetch_General(PyObject *d, osux_beatmap *m)
{
    if (!d)
        return;
    
    READ_STRING(m, AudioFilename,        d);
    READ_DOUBLE(m, AudioLeadIn,          d);
    READ_DOUBLE(m, PreviewTime,          d);
    READ_DOUBLE(m, Countdown,            d);
    READ_STRING(m, SampleSet,            d);
    READ_DOUBLE(m, StackLeniency,        d);
    READ_INT(   m, Mode,                 d);
    READ_INT(   m, LetterboxInBreaks,    d);
    READ_INT(   m, WidescreenStoryboard, d);

    // UseSkinSprites
    // StoryfireInFront
}

static void map_fetch_Editor(PyObject *d, osux_beatmap *m)
{
    if (!d)
	return;
    PyObject *v = NULL;

    v = PyDict_GetItemString(d, "Bookmarks");
    if (v) {
	m->bkmkc = (uint32_t) PyList_Size(v);
	m->Bookmarks = malloc(sizeof (*m->Bookmarks) * m->bkmkc);
	for (unsigned int i = 0; i < m->bkmkc; ++i)
	    m->Bookmarks[i] = (uint32_t) PyInt_AsLong(PyList_GetItem(v, i));
    }
    
    READ_DOUBLE(m, DistanceSpacing, d);
    READ_INT(m,    BeatDivisor, d);
    READ_INT(m,    GridSize, d);
    READ_DOUBLE(m, TimelineZoom, d);
    
}

static void map_fetch_Metadata(PyObject *d, osux_beatmap *m)
{
    if (!d)
	return;

    READ_STRING(m, Title, d);
    READ_STRING(m, TitleUnicode, d);
    READ_STRING(m, Artist, d);
    READ_STRING(m, ArtistUnicode, d);
    READ_STRING(m, Creator, d);
    READ_STRING(m, Version, d);
    READ_STRING(m, Source, d);

    READ_INT(m, BeatmapID, d);
    READ_INT(m, BeatmapSetID, d);


    // tags
    PyObject *v = PyDict_GetItemString(d, "Tags");
    if (v) {
	m->tagc = (uint32_t) PyList_Size(v);
	m->Tags = malloc(sizeof (*m->Tags) * m->tagc);
	for (unsigned int i = 0; i < m->tagc; ++i)
	    m->Tags[i] = strdup(PyString_AsString(PyList_GetItem(v, i)));
    }
}

static void map_fetch_Difficulty(PyObject *d, osux_beatmap *m)
{
    if (!d)
	return;
    READ_DOUBLE(m, HPDrainRate, d);
    READ_DOUBLE(m, CircleSize, d);
    READ_DOUBLE(m, OverallDifficulty, d);
    READ_DOUBLE(m, ApproachRate, d);
    READ_DOUBLE(m, SliderMultiplier, d);
    READ_DOUBLE(m, SliderTickRate, d);
}

/******************************************************************************/

static void col_py_fetch(struct color *c, PyObject *p)
{
    PyObject *q = PyList_GetItem(p, 1);
    
    c->r = PyInt_AsLong(PyList_GetItem(q, 0));
    c->g = PyInt_AsLong(PyList_GetItem(q, 1));
    c->b = PyInt_AsLong(PyList_GetItem(q, 2)) ;
}

static void map_fetch_Colours(PyObject *d, osux_beatmap *m)
{
    if (!d)
	return;

    size_t size = PyList_Size(d);
    m->Colours = malloc(sizeof(*m->Colours) * size);
    m->colc = size;
    for (unsigned int i = 0; i < size; ++i) {
	PyObject *p = PyList_GetItem(d, i);
	col_py_fetch(&m->Colours[i], p);
    }
}

/******************************************************************************/

#define TP_READ_DOUBLE(tp, dict, str)		\
    {						\
	PyObject *tmp;				\
	tmp = PyDict_GetItemString(dict, #str);	\
	tp->str = PyFloat_AsDouble(tmp);	\
    }						\
    
#define TP_READ_INT(tp, dict, str)		\
    {						\
	PyObject *tmp;				\
	tmp = PyDict_GetItemString(dict, #str);	\
	tp->str = PyInt_AsLong(tmp);		\
    }						\
    

static void tp_py_fetch(struct timing_point *tp, PyObject *tp_dict)
{
    TP_READ_DOUBLE(tp, tp_dict, offset);
    TP_READ_DOUBLE(tp, tp_dict, mpb);
    TP_READ_INT(tp, tp_dict, time_signature);
    TP_READ_INT(tp, tp_dict, sample_type);
    TP_READ_INT(tp, tp_dict, sample_set);
    TP_READ_INT(tp, tp_dict, volume);
    TP_READ_INT(tp, tp_dict, uninherited);
    TP_READ_INT(tp, tp_dict, kiai);
}

static void map_fetch_TimingPoints(PyObject *d, osux_beatmap *m)
{
    if (!d)
	return;
    m->tpc = (uint32_t) PyList_Size(d);
    m->TimingPoints = malloc(sizeof (*m->TimingPoints) * m->tpc);
    for (unsigned int i = 0; i < m->tpc; ++i)
	tp_py_fetch(&m->TimingPoints[i], PyList_GetItem(d, i));
}

/******************************************************************************/


#define HO_READ_DOUBLE(tp, dict, str)		\
    {						\
	PyObject *tmp;				\
	tmp = PyDict_GetItemString(dict, #str);	\
	if (tmp)				\
	    tp->str = PyFloat_AsDouble(tmp);	\
	else					\
	    tp->str = 0.;			\
    }						\


#define HO_READ_INT(tp, dict, str)		\
    {						\
	PyObject *tmp;				\
	tmp = PyDict_GetItemString(dict, #str);	\
	if (tmp)				\
	    tp->str = PyInt_AsLong(tmp);	\
	else					\
	    tp->str = 0.;			\
    }						\


#define HO_READ_STRING(tp, dict, str)			\
    {							\
	PyObject *tmp;					\
	tmp = PyDict_GetItemString(dict, #str);		\
	if (tmp && PyString_Check(tmp))			\
	    tp->str = strdup(PyString_AsString(tmp));	\
	else						\
	    tp->str = NULL;				\
    }							\


static void ho_py_fetch(struct hit_object *ho, PyObject *ho_dict)
{
    HO_READ_INT(ho, ho_dict, x);
    HO_READ_INT(ho, ho_dict, y);
    HO_READ_INT(ho, ho_dict, offset);
    HO_READ_INT(ho, ho_dict, type);
    
    HO_READ_INT(ho, ho_dict, hs.sample);
    HO_READ_INT(ho, ho_dict, hs.additional);
    
    HO_READ_INT(ho, ho_dict, hs.st);
    HO_READ_INT(ho, ho_dict, hs.st_additional);
    HO_READ_INT(ho, ho_dict, hs.sample_set_index);
    HO_READ_INT(ho, ho_dict, hs.volume);
    HO_READ_STRING(ho, ho_dict, hs.sfx_filename);

    HO_READ_INT(ho, ho_dict, sli.type); 
    HO_READ_INT(ho, ho_dict, sli.repeat);
    HO_READ_DOUBLE(ho, ho_dict, sli.length);

    // SLIDER POINTS
    PyObject *tmp1 = PyDict_GetItemString(ho_dict, "sli.coord");
    if (tmp1) {
	ho->sli.point_count = PyList_Size(tmp1);
	ho->sli.pos = malloc(sizeof(*ho->sli.pos) * ho->sli.point_count);
	for (unsigned int i = 0; i < ho->sli.point_count; ++i) {
	    PyObject *tmp2 = PyList_GetItem(tmp1, i);
	    ho->sli.pos[i].x = PyInt_AsLong(PyList_GetItem(tmp2, 0));
	    ho->sli.pos[i].y = PyInt_AsLong(PyList_GetItem(tmp2, 1));
	}
    }

    // SLIDER ADDITIONAL
    HO_READ_INT(ho, ho_dict, sli.hs.additional);
    if (ho->sli.hs.additional) {
	PyObject *slisample = PyDict_GetItemString(ho_dict, "sli.hs.sample");
	PyObject *slihsadd = PyDict_GetItemString(ho_dict, "sli.hs.st_add");

	ho->sli.hs.dat = malloc(sizeof(*ho->sli.hs.dat) *
				(ho->sli.repeat + 1));

	if (slisample) {
	    for (unsigned int i = 0; i < (ho->sli.repeat + 1); ++i) {
		ho->sli.hs.dat[i].sample
		    = PyInt_AsLong(PyList_GetItem(slisample, i));
	    }
	}
	
	if (slihsadd) { // TODO: disable this for v9 by example check version
	    for (unsigned int i = 0; i < (ho->sli.repeat + 1); ++i) {
		PyObject *tmp = PyList_GetItem(slihsadd, i);
		ho->sli.hs.dat[i].st = 
		    PyInt_AsLong(PyList_GetItem(tmp, 0));
		ho->sli.hs.dat[i].st_additional = 
		    PyInt_AsLong(PyList_GetItem(tmp, 1));
	    }
	}
    }

    // spinner
    HO_READ_INT(ho, ho_dict, spi.end_offset);
}

static void map_fetch_HitObjects(PyObject *d, osux_beatmap *m)
{
    if (!d)
	return;
    m->hoc = (uint32_t) PyList_Size(d);
    m->HitObjects = malloc(sizeof (*m->HitObjects) * m->hoc);
    for (unsigned int i = 0; i < m->hoc; ++i)
	ho_py_fetch(&m->HitObjects[i], PyList_GetItem(d, i));
}

/******************************************************************************/

static void map_fetch_Events(PyObject *d, osux_beatmap *m)
{
    (void) m;
    
    if (!d)
	return;
}


/******************************************************************************/


#define FETCH_SECTION(sect, pyobjsrc, map)                      \
    do {                                                        \
        PyObject *pyobjtmp;                                     \
        pyobjtmp = PyDict_GetItemString(pyobjsrc, #sect);       \
        map_fetch_##sect(pyobjtmp, map);                        \
    } while (0)

static osux_beatmap *fetch_beatmap(const char *filename)
{
    
    PyObject *data = embed_python_funcall(
        "osux_parse", "parse", 1, (const char*[]) { filename });
    if (!data) {
	fputs("Error parsing with omp python module\n", stderr);
	return NULL;
    }
    osux_beatmap *m = calloc(sizeof(*m), 1);

    m->version = PyInt_AsLong(PyDict_GetItemString(data, "version"));
    m->bom = PyInt_AsLong(PyDict_GetItemString(data, "BOM"));
    
    FETCH_SECTION( General,      data, m);
    FETCH_SECTION( Editor,       data, m);
    FETCH_SECTION( Difficulty,   data, m);
    FETCH_SECTION( Metadata,     data, m);
    FETCH_SECTION( Events,       data, m);
    FETCH_SECTION( TimingPoints, data, m);
    FETCH_SECTION( Colours,      data, m);
    FETCH_SECTION( HitObjects,   data, m);

    Py_XDECREF(data);

    return  m;
}

/******************************************************************************/

void __export parser_py_init(register_plugin_t register_callback, 
                             const osux_beatmap *default_bm)
{
    struct osux_bm_parser_callback cb;
    cb.parse_beatmap = fetch_beatmap;
    register_callback(&cb);
    DEFAULT_MAP = *default_bm;
}
