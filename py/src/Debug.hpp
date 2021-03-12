/*
 * Debug.hpp
 *
 *  Created on: 20 Mar 2020
 *      Author: julianporter
 */

#ifndef DEBUG_HPP_
#define DEBUG_HPP_


#include <iostream>

namespace debug {

enum class Level : int {
		Off = 0,
		Basic = 1,
		High = 2,
		FullDiagnostic = 3
	};

class Debug {
public:

private:
	Level level;
	Level outLevel;

	static int value(const Level l);
	bool isActive() const;


public:

	Debug(const Level l = Level::Off);
	virtual ~Debug() = default;
	Debug(const Debug &other) = default;

	void outputLevel(const Level l);
	void eol();
	template<typename T>
	void say(T &&arg) const {
		if(isActive()) std::cerr << arg;
	}

	static Debug debug;
	static void set(const Level &l) { debug=Debug(l); }

};
template<typename T>
Debug & operator<<(Debug& d,T &&value) {
	d.say(value);
	return d;
}


}




#endif /* DEBUG_HPP_ */
