/*
 * rubber.cpp
 *
 *  Created on: 14 Nov 2019
 *      Author: julianporter
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <sstream>
#include <iostream>
#include <stdexcept>

#define PY_ARRAY_UNIQUE_SYMBOL rubberband_ARRAY_API
#include <arrayobject.h>

#include "stretch.hpp"
#include "numpy.hpp"


static const bool Debug = false;
static PyObject *rubberbandError;

const char* ModuleName="rubberband";
const char* ErrorName="RubberBandError";
static char *Keywords[]={"data","format","rate","ratio","crispness","formants","precise",NULL};




static PyObject * stretch(PyObject *self, PyObject *args, PyObject *keywds) {
	PyTransformer::Debug=Debug;
	PyObject *stream;
	long sampleRate=64000;
	double ratio=1.0;
	int crispness=5;
	int precise=0;
	int formants=0;
	int fmt = NPY_FLOAT;

	if(!PyArg_ParseTupleAndKeywords(args,keywds,"O|ildipp",Keywords,
			&stream,&fmt,&sampleRate,&ratio,&crispness,&formants,&precise)) { return NULL; }

	try {
		PyTransformer transformer(stream,fmt,sampleRate,ratio,crispness,precise,formants);
		PyObject *o = transformer();
		Py_INCREF(o);
		return (PyObject *)o;
	}
	catch(std::exception &e) {
		PyErr_SetString(rubberbandError,e.what());
		if(Debug) std::cerr << e.what();
		return nullptr;
	}

}

static struct PyMethodDef methods[] = {
		{"stretch",(PyCFunction) stretch, METH_VARARGS | METH_KEYWORDS, "Stretch audio stream"},
		{NULL, NULL, 0, NULL}
};

static struct PyModuleDef module = {
		PyModuleDef_HEAD_INIT,
		ModuleName,
		"",			/// Documentation string
		-1,			/// Size of state (-1 if in globals)
		methods,
		NULL,		/// Slots
		NULL,		/// traverse
		NULL,		/// clear
		NULL		/// free
};

PyMODINIT_FUNC PyInit_rubberband(void) {
	PyObject *m = PyModule_Create(&module);
	if(m==NULL) return NULL;
	import_array();
	try {
		std::stringstream s;
		s << ModuleName << "." << ErrorName;
		rubberbandError=PyErr_NewException(s.str().c_str(),NULL,NULL);
		if(rubberbandError==NULL) throw std::runtime_error("Cannot allocate RubberbandError");
		Py_INCREF(rubberbandError);
		auto result=PyModule_AddObject(m,ErrorName,rubberbandError);
		if(result<0) throw std::runtime_error("Cannot attach RubberbandError to module");

		for(auto it=PyTransformer::formatNames.begin();it!=PyTransformer::formatNames.end();it++) {
			auto result=PyModule_AddIntConstant(m,it->second.c_str(),it->first);
			if(result<0) throw std::runtime_error("Cannot attach type formats to module");
		}

#ifdef MODULE_VERSION
		PyModule_AddStringConstant(m,"__version__",MODULE_VERSION);
#endif

		return m;
	}
	catch(std::exception &e) {
		PyErr_SetString(PyExc_ImportError,e.what());
		return NULL;
	}
}




