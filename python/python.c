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
#include <python2.7/Python.h>

__attribute__((constructor))
static void embed_python_init(void)
{
    Py_Initialize();    

    PyObject *sysPath = PySys_GetObject((char*)"path");
    PyObject *pPath = PyString_FromString(PYTHON_PATH);

    PyList_Append(sysPath, pPath);
    Py_DECREF(pPath);
}

__attribute__((destructor))
static void embed_python_exit(void)
{
    Py_Finalize();
}

PyObject *embed_python_funcall(
    const char *module, const char *fun, int argc, const char *argv[])
{			   
    PyObject *pName, *pModule, *pFunc;
    PyObject *pArgs, *pValue = NULL;

    pName = PyString_FromString(module);
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, fun);
        /* pFunc is a new reference */

        if (pFunc && PyCallable_Check(pFunc)) {
	    pArgs = PyTuple_New(argc);
            for (int i = 0; i < argc; ++i) {
                pValue = PyString_FromString(argv[i]);
                if (!pValue) {
                    Py_DECREF(pArgs);
                    Py_DECREF(pModule);
                    fprintf(stderr, "Cannot convert argument\n");
                    return NULL;
                }
                /* pValue reference stolen here: */
                PyTuple_SetItem(pArgs, i, pValue);
            }
	    
	    pValue = PyObject_CallObject(pFunc, pArgs);
	    Py_DECREF(pArgs);
	    
	    if (!pValue) {
                Py_DECREF(pFunc);
                Py_DECREF(pModule);
                PyErr_Print();
                fprintf(stderr,"Call failed\n");
		return NULL;
            }
        } else {
            if (PyErr_Occurred())
                PyErr_Print();
            fprintf(stderr, "Cannot find function \"%s\"\n", fun);
        }
        Py_XDECREF(pFunc);
        Py_DECREF(pModule);
    } else {
        PyErr_Print();
	return NULL;
    }
    return pValue;
}


#ifdef USE_PYTHON_CUSTOM_PATH

#ifndef PYTHON_CUSTOM_PATH
#error PYTHON_CUSTOM_PATH must be set when USE_PYTHON_CUSTOM_PATH is used
#endif // PYTHON_CUSTOM_PATH

const char *
Py_GetPath(void)
{
    return PYTHON_CUSTOM_PATH;
}

#endif // USE_PYTHON_CUSTOM_PATH
