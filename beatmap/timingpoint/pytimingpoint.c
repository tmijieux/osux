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

#include <python2.7/Python.h>
#include "timing_point.h"
#include "pytimingpoint.h"

#define READ_DOUBLE(tp, dict, str)		\
    {						\
	PyObject *tmp;				\
	tmp = PyDict_GetItemString(dict, #str);	\
	tp->str = PyFloat_AsDouble(tmp);	\
    }						\
    
#define READ_INT(tp, dict, str)			\
    {						\
	PyObject *tmp;				\
	tmp = PyDict_GetItemString(dict, #str);	\
	tp->str = PyInt_AsLong(tmp);		\
    }						\
    

void tp_py_parse(struct timing_point *tp, PyObject *tp_dict)
{
    READ_DOUBLE(tp, tp_dict, offset);
    READ_DOUBLE(tp, tp_dict, mpb);
    READ_INT(tp, tp_dict, time_signature);
    READ_INT(tp, tp_dict, sample_type);
    READ_INT(tp, tp_dict, sample_set);
    READ_INT(tp, tp_dict, volume);
    READ_INT(tp, tp_dict, uninherited);
    READ_INT(tp, tp_dict, kiai);
}
