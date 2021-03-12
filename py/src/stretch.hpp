/*
 * main.hpp
 *
 *  Created on: 15 Nov 2019
 *      Author: julianporter
 */

#ifndef STRETCH_HPP_
#define STRETCH_HPP_

#include <vector>
#include <sndfile.h>
#include <rubberband/RubberBandStretcher.h>

using namespace RubberBand;
using RB = RubberBandStretcher;




class Stretch {
private:

	unsigned countOut=0;

	unsigned nFramesIn;
	unsigned nChannels;
	int sampleRate;

	std::vector<float> in;
	std::vector<float> out;

	RB stretcher;



	std::vector<float>::iterator it;
	std::vector<float> buffer;

	void processAvailable(const int available);

	void study();
	void process();

public:

	using Options = RB::Options;
	static Options makeOptions(const int crispness=5,const bool formant=false,const bool precise=true);

	Stretch(const unsigned frames,const unsigned channels,const int samplerate,const double ratio,const RB::Options opts = 0);
	Stretch(const SF_INFO &info,const double ratio,const RB::Options opts = 0);
	virtual ~Stretch() = default;

	std::vector<float> operator()(const std::vector<float> &input);
	std::vector<double> operator()(const std::vector<double> &input);
};



#endif /* STRETCH_HPP_ */
