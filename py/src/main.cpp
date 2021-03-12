/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Rubber Band Library
    An audio time-stretching and pitch-shifting library.
    Copyright 2007-2018 Particular Programs Ltd.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.

    Alternatively, if you have a valid commercial licence for the
    Rubber Band Library obtained by agreement with the copyright
    holders, you may redistribute and/or modify it under the terms
    described in that licence.

    If you wish to distribute code using the Rubber Band Library
    under terms other than those of the GNU General Public License,
    you must obtain a valid commercial licence before doing so.
*/

#include <rubberband/RubberBandStretcher.h>

#include <iostream>
#include <sndfile.h>
#include <cmath>
#include <time.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include <getopt.h>
#include <unistd.h>

#include <fstream>

#include "stretch.hpp"
#include "Debug.hpp"


using namespace RubberBand;



static const int ibs=1024;
static struct option opts[] = {
		{ "crispness", required_argument, 0, 'c' },
		{ "formants",  no_argument,       0, 'f' },
		{ "precise",   no_argument,       0, 'p' },
		{ "duration",  required_argument, 0, 'd' },
		{ 0,0,0,0 }
};

int main(int argc, char **argv)
{
	int crispness = 5;
	bool formants = false;
	bool precise = false;
	double duration = -1;

	opterr = 0;  // quiet option scanning
	int optionIndex = 0;
	while(true) {
		auto c=getopt_long(argc,argv,"c::fpd:",opts,&optionIndex);
		if(c == -1) break;

		switch(c) {
		case 'f':
			formants=true;
			break;
		case 'p':
			precise=true;
			break;
		case 'd':
			duration=std::stod(optarg);
			break;
		case 'c':
			crispness=std::stoi(optarg);
			break;
		}
	}

	if(crispness>6||crispness<0) {
		std::cerr << "Error: crispness must be between 0 and 6" << std::endl;
		return 2;
	}
	if(duration<=0.0) {
		std::cerr << "Error: duration must be non-negative float" << std::endl;
		return 2;
	}

	auto o=optind;
    const char *inFile = strdup(argv[o]);
    const char *outFile= strdup(argv[o+1]);


    std::cout << "processing " << inFile << " -> " << outFile << " with duration " << duration << std::endl;
    std::cout << "crispness = " << crispness << std::endl;
    std::cout << "formants  = " << formants << std::endl;
    std::cout << "precise   = " << precise << std::endl;

    SF_INFO sfinfo;
    memset(&sfinfo, 0, sizeof(SF_INFO));

    auto sndfile = sf_open(inFile, SFM_READ, &sfinfo);
    if (!sndfile) {
    	std::cerr << "ERROR: Failed to open input file \"" << inFile << "\": " << sf_strerror(sndfile) << std::endl;
    	return 1;
    }
    if (sfinfo.frames == 0 || sfinfo.samplerate == 0) {
        std::cerr << "ERROR: File lacks frame count or sample rate in header, cannot use --duration" << std::endl;
        return 1;
    }
    if (sfinfo.channels > 1) {
    	std::cerr << "ERROR: This library only works with mono files" << std::endl;
    	return 1;
    }

    auto induration = double(sfinfo.frames) / double(sfinfo.samplerate);
    auto ratio = (induration != 0.0) ? duration / induration : 0.0;

    std::vector<float> in;
    auto buffer = new float[ibs];
    int frame = 0;
    while (frame < sfinfo.frames) {
        auto count = sf_readf_float(sndfile, buffer, ibs);
        if (count<=0) break;
        in.insert(in.end(),buffer,buffer+count);
        frame+=count;
    }
    sf_close(sndfile);
    delete[] buffer;

    std::cout << "In:" << std::endl;
    for(auto i=0;i<100;i++) {
    	std::cout << in[i] << " ";
    	if(5== i%6) std::cout << std::endl;
    }

    //debug::set(debug::Level::Basic);

    auto options=Stretch::makeOptions(crispness,formants,precise);
    Stretch st(sfinfo,ratio,options);
    auto out = st(in);

    std::cout << std::endl << "Out:" << std::endl;
    for(auto i=0;i<100;i++) {
        	std::cout << out[i] << " ";
        	if(5== i%6) std::cout << std::endl;
        }

    SF_INFO sfinfoOut;
    sfinfoOut.channels = sfinfo.channels;
    sfinfoOut.format = sfinfo.format;
    sfinfoOut.frames = out.size();
    sfinfoOut.samplerate = sfinfo.samplerate;
    sfinfoOut.sections = sfinfo.sections;
    sfinfoOut.seekable = sfinfo.seekable;
    auto sndfileOut = sf_open(outFile, SFM_WRITE, &sfinfoOut) ;
    if (!sndfileOut) {
    	std::cerr << "ERROR: Failed to open output file \"" << outFile << "\" for writing: "
    			<< sf_strerror(sndfileOut) << std::endl;
        return 1;
    }

    int offset=0;
    while(offset<out.size()) {
    	int set=std::min((int)out.size()-offset,ibs);
    	auto wrote=sf_writef_float(sndfileOut, out.data()+offset, set);
    	if(wrote<=0) break;
    	offset+=wrote;
    }
    sf_close(sndfileOut);

    return 0;
}



