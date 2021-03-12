/*
 * Debug.cpp
 *
 *  Created on: 20 Mar 2020
 *      Author: julianporter
 */

#include "Debug.hpp"
#include <rubberband/RubberBandStretcher.h>

using namespace RubberBand;
using RB = RubberBandStretcher;


namespace debug {
Debug::Debug(const Level l) : level(l), outLevel(l) {
	RB::setDefaultDebugLevel(value(l));
};

int Debug::value(const Level l) { return static_cast<int>(l); }

bool Debug::isActive() const { return (value(outLevel) >= value(level)) && (level != Level::Off); }

void Debug::outputLevel(const Level l) {
		outLevel=l;
	}
void Debug::eol() {
		if(isActive()) {
			std::cerr << std::endl;
			outLevel=level;
		}
	}

Debug Debug::debug = Debug();

}
