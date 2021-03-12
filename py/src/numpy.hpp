/*
 * numpy.hpp
 *
 *  Created on: 5 Apr 2020
 *      Author: julianporter
 */

#ifndef SRC_NUMPY_HPP_
#define SRC_NUMPY_HPP_

#include <Python.h>
#include <vector>
#include <string>
#include <map>

//
// np.dtype <-> PyArray_Descr
// PyArray_DescrFromType(type).num == type where type is one of NPY_INT16, etc
//

enum class Content {
	Array, List, Buffer
};

using vect_t = std::vector<double>;
using range_t = std::pair<double,double>;




class PyTransformer {
	
private:
	
	static std::map<int,float> casts;
	static std::map<int,range_t> ranges;
	
	
	long sampleRate;
	double ratio ;
	int crispness ;
	bool precise ;
	bool formants ;
	int format;
	
	Content mode;
	
	vect_t in;
	vect_t out;
	
	void numpyToVector(PyObject *obj);
	PyObject *vectorToNumpy();
	void listToVector(PyObject *obj);
	PyObject *vectorToList();
	void bufferToVector(PyObject *obj);
	PyObject *vectorToBuffer();
	
	void unpack(PyObject *stream);
	PyObject *pack();
		
public:
	static bool Debug;
	static std::map<int,std::string> formatNames;

	PyTransformer(PyObject *stream, const int format_,const long sampleRate_=48000,
		const double ratio_=1.0, const int crispness_=5, const int precise_=1, const int formants_=0);
	virtual ~PyTransformer() = default;
	
	PyObject * operator()();
	
	
};

#endif /* SRC_NUMPY_HPP_ */
