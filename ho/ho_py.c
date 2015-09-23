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

#include <stdio.h>
#include <stdlib.h>
#include <python2.7/Python.h>

#include "hit_object.h"

#define READ_DOUBLE(tp, dict, str)		\
    {						\
	PyObject *tmp;				\
	tmp = PyDict_GetItemString(dict, #str);	\
	if (tmp)				\
	    tp->str = PyFloat_AsDouble(tmp);	\
	else					\
	    tp->str = 0.;			\
    }						\


#define READ_INT(tp, dict, str)			\
    {						\
	PyObject *tmp;				\
	tmp = PyDict_GetItemString(dict, #str);	\
	if (tmp)				\
	    tp->str = PyInt_AsLong(tmp);	\
	else					\
	    tp->str = 0.;			\
    }						\


#define READ_STRING(tp, dict, str)			\
    {							\
	PyObject *tmp;					\
	tmp = PyDict_GetItemString(dict, #str);		\
	if (tmp && PyString_Check(tmp))			\
	    tp->str = strdup(PyString_AsString(tmp));	\
	else						\
	    tp->str = NULL;				\
    }							\


void ho_py_parse(struct hit_object *ho, PyObject *ho_dict)
{
    READ_INT(ho, ho_dict, x);
    READ_INT(ho, ho_dict, y);
    READ_INT(ho, ho_dict, offset);
    READ_INT(ho, ho_dict, type);
    
    READ_INT(ho, ho_dict, hs.sample);
    READ_INT(ho, ho_dict, hs.additional);
    
    READ_INT(ho, ho_dict, hs.st);
    READ_INT(ho, ho_dict, hs.st_additional);
    READ_INT(ho, ho_dict, hs.sample_set_index);
    READ_INT(ho, ho_dict, hs.volume);
    READ_STRING(ho, ho_dict, hs.sfx_filename);

    READ_INT(ho, ho_dict, sli.type); 
    READ_INT(ho, ho_dict, sli.repeat);
    READ_DOUBLE(ho, ho_dict, sli.length);

    // SLIDER POINTS
    PyObject *tmp1 = PyDict_GetItemString(ho_dict, "sli.coord");
    if (tmp1) {
	ho->sli.point_count = PyList_Size(tmp1);
	ho->sli.pos = malloc(sizeof(*ho->sli.pos) * ho->sli.point_count);
	for (int i = 0; i < ho->sli.point_count; ++i) {
	    PyObject *tmp2 = PyList_GetItem(tmp1, i);
	    ho->sli.pos[i].x = PyInt_AsLong(PyList_GetItem(tmp2, 0));
	    ho->sli.pos[i].y = PyInt_AsLong(PyList_GetItem(tmp2, 1));
	}
    }

    // SLIDER ADDITIONAL
    READ_INT(ho, ho_dict, sli.hs.additional);
    if (ho->sli.hs.additional) {
	PyObject *slisample = PyDict_GetItemString(ho_dict, "sli.hs.sample");
	PyObject *slihsadd = PyDict_GetItemString(ho_dict, "sli.hs.st_add");

	ho->sli.hs.dat = malloc(sizeof(*ho->sli.hs.dat) *
				(ho->sli.repeat + 1));

	if (slisample) {
	    for (int i = 0; i < (ho->sli.repeat + 1); ++i) {
		ho->sli.hs.dat[i].sample
		    = PyInt_AsLong(PyList_GetItem(slisample, i));
	    }
	}
	if (slihsadd) { // TODO: disable this for v9 by example check version
	    for (int i = 0; i < (ho->sli.repeat + 1); ++i) {
		PyObject *tmp = PyList_GetItem(slihsadd, i);
		ho->sli.hs.dat[i].st = 
		    PyInt_AsLong(PyList_GetItem(tmp, 0));
		ho->sli.hs.dat[i].st_additional = 
		    PyInt_AsLong(PyList_GetItem(tmp, 1));
	    }
	}
    }
    
    READ_INT(ho, ho_dict, spi.end_offset);
}
