/*
 * stretch.cpp
 *
 *  Created on: 15 Nov 2019
 *      Author: julianporter
 */


#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>
#include <map>
//#include "Debug.hpp"

#include "./stretch.hpp"

static const unsigned ibs=1024;

class StretchBuffer {
public:
	std::vector<float> buffer;
	unsigned long  chunk;
	long offset;
	unsigned long n;
	unsigned long remaining;


public:
	StretchBuffer(const std::vector<float> &b_,const unsigned c_ = 1024) : buffer(b_), chunk(c_), offset(-chunk), n(buffer.size()), remaining(n) {};
	virtual ~StretchBuffer() = default;

	void reset() {
		offset=0;
		remaining=n;
	}
	bool step() {
		offset+=chunk;
		remaining=n-offset;
		return offset<n;
	}


	unsigned long size() const { return std::min<unsigned>(chunk,remaining); }
	float * operator *() { return buffer.data()+offset; }
	operator bool() const { return chunk >= remaining; }

};




	static const std::map<int,Stretch::Options> OptionMap = {
		{ 0, RB::OptionWindowLong | RB::OptionPhaseIndependent  | RB::OptionTransientsSmooth},
		{ 1, RB::OptionWindowLong | RB::OptionPhaseIndependent  | RB::OptionDetectorSoft},
		{ 2, RB::OptionPhaseIndependent | RB::OptionTransientsSmooth},
		{ 3, RB::OptionTransientsSmooth},
		{ 4, RB::OptionTransientsMixed},
		{ 5, RB::OptionPhaseLaminar | RB::OptionTransientsCrisp | RB::OptionDetectorCompound },
		{ 6, RB::OptionWindowShort | RB::OptionPhaseIndependent }
	};

	Stretch::Options Stretch::makeOptions(const int crispness,const bool formant,const bool precise) {
		if(crispness<0||crispness>6) throw std::runtime_error("Crispness out of range");
		auto option=OptionMap.at(crispness);
		if(formant) option |= RB::OptionFormantPreserved;
		if(precise) option |= RB::OptionStretchPrecise;
		return option;
	}

Stretch::Stretch(const unsigned frames,const unsigned channels,const int samplerate,const double ratio,const RB::Options opts) :
		nFramesIn(frames), nChannels(channels), sampleRate(samplerate),
		in(nChannels*nFramesIn,0.0), out(),
		stretcher(sampleRate,nChannels,opts,ratio,1.0), buffer() {}
Stretch::Stretch(const SF_INFO &info,const double ratio,const RB::Options opts) :
		Stretch(info.frames,info.channels,info.samplerate,ratio,opts) {}

std::vector<double> Stretch::operator()(const std::vector<double> &input) {
	std::vector<float> i(input.size(),0);
	std::transform(input.begin(),input.end(),i.begin(),[](double x) { return (float)x; });

	(*this)(i);
	std::vector<double> o(out.size(),0);
	std::transform(out.begin(),out.end(),o.begin(),[](float x) { return (double)x; });
	return o;
}
std::vector<float> Stretch::operator()(const std::vector<float> &input) {
	in.assign(input.begin(),input.end());
	countOut=0;
	out.clear();

	study();

	in.assign(input.begin(),input.end());
	process();

	std::transform(out.begin(),out.end(),out.begin(),[](const float x) {
		return std::min(1.0f,std::max(-1.0f,x));
	});
	return out;
}



void Stretch::study() {
	//debug::Debug::debug << "Studying "  << in.size() << " samples "; debug::Debug::debug.eol();

	stretcher.setExpectedInputDuration(in.size());
	StretchBuffer buffer(in,ibs);
	while(buffer.step()) {
		//debug::Debug::debug.outputLevel(debug::Level::High);
		//debug::Debug::debug << "  " << buffer.size() << " samples at offset " << buffer.offset << " (final: " << (bool)buffer << ")";  debug::Debug::debug.eol();
		auto ibuf=*buffer;
		stretcher.study(&ibuf,buffer.size(),buffer);
	}


}

void Stretch::process() {

	//debug::Debug::debug << "Processing " << in.size() << " samples "; debug::Debug::debug.eol();
	StretchBuffer buffer(in,ibs);
	while(buffer.step()) {
		auto ibuf=*buffer;
		stretcher.process(&ibuf,buffer.size(),buffer);
		int available=stretcher.available();
		if(available>0) processAvailable(available);
	}

	int available=stretcher.available();
	while (available>= 0) {
		if (available > 0) {
			processAvailable(available);
		} else {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
		available=stretcher.available();
	}

	//debug::Debug::debug << "Processing completed"; debug::Debug::debug.eol();
}

void Stretch::processAvailable(const int available) {
	//debug::Debug::debug.outputLevel(debug::Level::High);
	//debug::Debug::debug << "Retrieving " << available; debug::Debug::debug.eol();

	buffer.assign(available,0);
	auto p=buffer.data();
	stretcher.retrieve(&p, available);
	countOut += available;
	out.insert(out.end(),buffer.begin(),buffer.end());
}


