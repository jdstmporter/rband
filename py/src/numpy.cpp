/*
 * numpy.cpp
 *
 *  Created on: 5 Apr 2020
 *      Author: julianporter
 */

#include "numpy.hpp"
#include <stdexcept>
#include <map>
#include <iostream>
#include <cmath>
#include "stretch.hpp"


#define PY_ARRAY_UNIQUE_SYMBOL rubberband_ARRAY_API
#define NO_IMPORT_ARRAY
#include <arrayobject.h>

double clip(const range_t &range,const double x) { return std::max(range.first,std::min(range.second,x)); }



Content discriminate(PyObject *o) {
	if(PyArray_Check(o)) {
		PyArrayObject *array=(PyArrayObject *)o;
		if(array!=nullptr && PyArray_NDIM(array)==1) return Content::Array;
	}
	else if(PyList_Check(o)) return Content::List;
	else if(PyBytes_Check(o)) return Content::Buffer;
	throw std::runtime_error("Input data must be of type np.array, list or bytes");
}

std::map<Content,std::string> names {
	{ Content::Array , "numpy" },
	{ Content::List, "list" },
	{ Content::Buffer, "bytes" }
};

std::map<int,std::string> PyTransformer::formatNames {
		{ NPY_FLOAT , "float32" },
		{ NPY_UINT8 , "uint8" },
		{ NPY_INT8 , "int8" },
		{ NPY_INT16 , "int16" },
		{ NPY_INT32 , "int32" }
};

bool PyTransformer::Debug = false;
std::map<int,float> PyTransformer::casts {
		{ NPY_FLOAT , 1.0 },
		{ NPY_UINT8 , 255.0 },
		{ NPY_INT8 , 128.0 },
		{ NPY_INT16 , 32768.0 },
		{ NPY_INT32 , 2147483648.0 }
};
std::map<int,range_t> PyTransformer::ranges {
		{ NPY_UINT8 , std::make_pair(0.0,255.0) },
		{ NPY_INT8 ,  std::make_pair(-128.0,127.0) },
		{ NPY_INT16 , std::make_pair(-32768.0,32767.0) },
		{ NPY_INT32 , std::make_pair(-2147483648.0,2147483647.0) }
};



void PyTransformer::numpyToVector(PyObject *obj) {
	PyObject *inter=PyArray_Cast((PyArrayObject *)obj,NPY_FLOAT);
	PyObject *list=PyArray_ToList((PyArrayObject *)inter);
	listToVector(list);
}

PyObject *PyTransformer::vectorToNumpy() {
	PyObject *list=vectorToList();
	PyObject *inter=PyArray_FROM_OT(list,NPY_FLOAT);
	return PyArray_Cast((PyArrayObject *)inter,format);
}

void PyTransformer::bufferToVector(PyObject *buffer) {
	auto dtype=PyArray_DescrFromType(format);
	auto n=PyBytes_Size(buffer);
	auto ptr=PyBytes_AsString(buffer);
	PyObject *inter = PyArray_FromString(ptr,n,dtype,-1,NULL);
	return numpyToVector(inter);
}

PyObject *PyTransformer::vectorToBuffer() {
	PyArrayObject * inter = (PyArrayObject *)vectorToNumpy();
	return PyArray_ToString(inter,NPY_CORDER);
}

void PyTransformer::listToVector(PyObject *obj) {
	if(!PyList_Check(obj)) throw std::runtime_error("Object is not a list");
	long n = PyList_Size(obj);
	if(n<0) throw std::runtime_error("Object is not a list");

	in.assign(n,0.0);
	for(auto i=0;i<n;i++) {
		auto item = PyList_GetItem(obj,i);
		if(item==NULL) throw std::runtime_error("Cannot read list entries");
		if(!PyFloat_Check(item)) throw std::runtime_error("Non-float list entries");
		in[i]=PyFloat_AS_DOUBLE(item);
	}
}

PyObject *PyTransformer::vectorToList() {
	unsigned long n = out.size();
	auto obj=PyList_New(n);
	if(obj==nullptr) throw std::runtime_error("Cannot allocate list");
	if(format==NPY_FLOAT) {
		for(unsigned long i=0;i<n;i++) {
			auto val = PyFloat_FromDouble(out[i]);
			PyList_SetItem(obj,i,val);
		}
	}
	else {
		for(unsigned long i=0;i<n;i++) {
			auto val =PyLong_FromLong((long)out[i]);
			PyList_SetItem(obj,i,val);
		}
	}
	return obj;
}

void PyTransformer::unpack(PyObject *stream) {
	switch(mode) {
		case Content::List:
			listToVector(stream);
			break;
		case Content::Array:
			numpyToVector(stream);
			break;
		case Content::Buffer:
			bufferToVector(stream);
			break;
		}
	if(format!=NPY_FLOAT) {
		auto scale = 1.0/PyTransformer::casts.at(format);
		auto range = PyTransformer::ranges.at(format);

		if(Debug) {
			std::cout << "Scaler: " << scale << std::endl;
			std::cout << "Range : " << range.first << " - " << range.second << std::endl;
			std::cout << "Unscaled In:" << std::endl;
			for(auto i=0;i<25;i++) {
				std::cout << in[i] << " ";
				if(5== i%6) std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		std::transform(in.begin(),in.end(),in.begin(),[scale,range](double x){ return x*clip(range,scale); });
		if(Debug) {
			std::cout << "Scaled In:" << std::endl;
			for(auto i=0;i<25;i++) {
				std::cout << in[i] << " ";
				if(5== i%6) std::cout << std::endl;
			}
			std::cout << std::endl;
		}
	}
}

PyObject *PyTransformer::pack() {
	if(format!=NPY_FLOAT) {
		auto scale = PyTransformer::casts.at(format);
		auto range = PyTransformer::ranges.at(format);

		if(Debug) {
			std::cout << "Unscaled out:" << std::endl;
			for(auto i=0;i<25;i++) {
				std::cout << out[i] << " ";
				if(5== i%6) std::cout << std::endl;
			}
			std::cout << std::endl;
		}

		std::transform(out.begin(),out.end(),out.begin(),[scale,range](double x){ return clip(range,round(x*scale)); });

		if(Debug) {
				std::cout << "Scaler: " << scale << std::endl;
				std::cout << "Range : " << range.first << " - " << range.second << std::endl;
				std::cout << "Scaled and truncated out:" << std::endl;
				for(auto i=0;i<25;i++) {
					std::cout << out[i] << " ";
					if(5== i%6) std::cout << std::endl;
				}
				std::cout << std::endl;
			}
	}
	switch(mode) {
		case Content::List:
			return vectorToList();
		case Content::Array:
			return vectorToNumpy();
		case Content::Buffer:
			return vectorToBuffer();
	}
	return NULL;
}



PyTransformer::PyTransformer(PyObject *stream,const int format_, const long sampleRate_,
		const double ratio_, const int crispness_, const int precise_, const int formants_) :
		sampleRate(sampleRate_), ratio(ratio_), crispness(crispness_), precise(precise_!=0),
		formants(formants_!=0), in(), out() {

	mode=discriminate(stream);
	if(mode==Content::Array) {
		auto dtype = PyArray_DTYPE((PyArrayObject *)stream);
		format=dtype->type_num;
	}
	else {
		format=format_;
	}

	if(Debug) {
			std::cout << "Mode is " << names[mode] << std::endl;
			std::cout << "Format is " << formatNames[format] << std::endl;
		}

	unpack(stream);
}

PyObject * PyTransformer::operator()() {
	auto option=Stretch::makeOptions(crispness,formants,precise);
	option |= RB::OptionThreadingNever;
	if(Debug) {
		std::cout << "Crispness is " << crispness << ", formants is " << formants << ", precise is " << precise << std::endl;
		std::cout << "Option is " << std::hex << option << std::dec << std::endl;

		std::cout << "N = " << in.size() << std::endl;
		std::cout << "rate = " << sampleRate << std::endl;
		std::cout << "ratio = " << ratio << std::endl;
		std::cout << "option = " << option << std::endl;

		std::cout << "In raw:" << std::endl;
		for(auto i=0;i<100;i++) {
			std::cout << in[i] << " ";
			if(5== i%6) std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	Stretch st(in.size(),1,sampleRate,ratio,option);
	out = st(in);

	if(Debug) {
		std::cout << "Out:" << std::endl;
		for(auto i=0;i<100;i++) {
			std::cout << out[i] << " ";
			if(5== i%6) std::cout << std::endl;
		}
		std::cout << std::endl;
	}

	return pack();
}



