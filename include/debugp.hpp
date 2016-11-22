/*
* Copyright (C) 2015-2016, Tino Didriksen <mail@tinodidriksen.com>
*
* This file is part of Spellers
*
* Spellers is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Spellers is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Spellers.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#ifndef DEBUGP_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define DEBUGP_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include <fstream>
#include <string>

#ifndef NDEBUG
#undef debugp
#undef p
extern std::ofstream debug;
extern thread_local size_t debugd;
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

	template<typename T, typename U>
	void operator()(const T& t, const U& u) {
		debug << std::string(debugd, '\t') << t << ' ' << u << std::endl;
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

	template<typename U>
	void operator()(const std::wstring& t, const U& u) {
		debug << std::string(debugd, '\t');
		for (size_t i = 0; i<t.size(); ++i) {
			if (t[i] < 255) {
				char c = static_cast<char>(t[i]);
				debug.write(&c, 1);
			}
		}
		debug << ' ' << u << std::endl;
	}
};
#else
	#undef debugp
	#undef p
	#define debugp
	#define p
#endif

#endif
