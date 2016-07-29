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

#include "./python_internal.h"
#include "osux/error.h"
#include "osux/compiler.h"

static void embed_python_exit(void)
{
    Py_Finalize();
}

INITIALIZER(embed_python_init)
{
    Py_Initialize();

    PyObject *sysPath = PySys_GetObject((char*) "path");
    PyObject *pPath = PyString_FromString(MODULE_LOAD_PATH);

    PyList_Append(sysPath, pPath);
    Py_DECREF(pPath);
    atexit(&embed_python_exit);
}

static PyObject *python_funcall_pyobj_callable(
    PyObject *pFunc, int argc, PyObject *argv[])
{
    PyObject *pArgs = NULL, *pValue = NULL;
    pArgs = PyTuple_New(argc);
    for (int i = 0; i < argc; ++i)
        PyTuple_SetItem(pArgs, i, argv[i]);

    pValue = PyObject_CallObject(pFunc, pArgs);
    Py_DECREF(pArgs);

    if (!pValue) {
        PyErr_Print();
        osux_error("Call failed\n");
        return NULL;
    }
    return pValue;
}

static PyObject *python_funcall_pyobj_module(
    PyObject *pModule, const char *name, int argc, PyObject *argv[])
{
    PyObject *pFunc, *pValue = NULL;
    pFunc = PyObject_GetAttrString(pModule, name);
    if (pFunc != NULL && PyCallable_Check(pFunc)) {
        pValue = python_funcall_pyobj_callable(pFunc, argc, argv);
    } else {
        if (PyErr_Occurred())
            PyErr_Print();
        osux_error("Cannot find function \"%s\"\n", name);
    }
    Py_XDECREF(pFunc);
    return pValue;
}

PyObject *python_funcall_name(
    const char *module, const char *fun, int argc, PyObject *argv[])
{
    PyObject *pName, *pModule, *pValue = NULL;

    pName = PyString_FromString(module);
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    if (pModule != NULL)
        pValue = python_funcall_pyobj_module(pModule, fun, argc, argv);
    else
        PyErr_Print();
    Py_XDECREF(pModule);
    return pValue;
}

PyObject *python_funcall_name_string_args(
    const char *module, const char *fun, int argc, const char *argv[])
{
    PyObject **pArgv, *pValue = NULL;

    pArgv = malloc(sizeof*pArgv * argc);
    for (int i = 0; i < argc; ++i)
        pArgv[i] = PyString_FromString(argv[i]);
    pValue = python_funcall_name(module, fun, argc, pArgv);

    for (int i = 0; i < argc; ++i)
        Py_XDECREF(pArgv[i]);
    free(pArgv);
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
