/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#pragma once
#ifndef DEBUGP_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define DEBUGP_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include <fstream>
#include <string>

#undef NDEBUG
#ifndef NDEBUG
extern std::ofstream debug;
extern size_t debugd;
struct debugp {
	std::string msg;

	debugp(const std::string& msg) : msg(msg) {
		debug << std::string(debugd, '\t') << msg << std::endl;
		++debugd;
	}

	~debugp() {
		--debugd;
		debug << std::string(debugd, '\t') << '~' << msg << std::endl;
	}

	template<typename T>
	void operator()(const T& t) {
		debug << std::string(debugd, '\t') << t << std::endl;
	}

	void operator()(const std::wstring& t) {
		debug << std::string(debugd, '\t');
		for (size_t i = 0; i<t.size(); ++i) {
			if (t[i] < 255) {
				char c = static_cast<char>(t[i]);
				debug.write(&c, 1);
			}
		}
		debug << std::endl;
	}
};
#else
	#define debugp
	#define p
#endif

#endif
