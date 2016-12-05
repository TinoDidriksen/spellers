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
#ifndef SHARED_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define SHARED_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#ifdef _MSC_VER
	// warning C4512: assignment operator could not be generated
	#pragma warning (disable: 4512)
	// warning C4456: declaration hides previous local declaration
	#pragma warning (disable: 4456)
	// warning C4458: declaration hides class member
	#pragma warning (disable: 4458)
	// warning C4459: declaration hides global declaration
	#pragma warning (disable: 4459)
#endif

#include <fstream>
#include <string>
#include <map>
#include <cctype>
#include <algorithm>
#include <cstdint>
#include <utf8.h>

#ifdef _WIN32
	#include <windows.h>
#else
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif
	#include <dlfcn.h>
	#include <cstdio>
#endif

#include <debugp.hpp>

const size_t PROOF_VERSION_MAJOR = 1;
const size_t PROOF_VERSION_MINOR = 0;
const size_t PROOF_VERSION_PATCH = 0;

inline std::string trim(std::string str) {
	while (!str.empty() && (str.back() == 0 || std::isspace(str.back()))) {
		str.pop_back();
	}
	while (!str.empty() && std::isspace(str[0])) {
		str.erase(str.begin());
	}
	return str;
}

inline bool read_conf(std::map<std::string,std::string>& conf) {
	debugp p(__FUNCTION__);

	std::string path;
#ifdef _WIN32
	path.resize(MAX_PATH + 1);
	MEMORY_BASIC_INFORMATION mbiInfo = { 0 };
	if (VirtualQuery(reinterpret_cast<void*>(read_conf), &mbiInfo, sizeof(mbiInfo))) {
		GetModuleFileNameA((HMODULE)(mbiInfo.AllocationBase), &path[0], MAX_PATH);
	}
#else
	Dl_info dl_info = {};
	dladdr(reinterpret_cast<void*>(read_conf), &dl_info);
	path.assign(dl_info.dli_fname);
#endif
	if (path[0] == 0) {
		p("path was empty");
		return false;
	}
	while (!path.empty() && path.back() != '/' && path.back() != '\\') {
		path.pop_back();
	}

	conf["PATH"] = path;

	std::string line;
	std::ifstream inif(path + "speller.ini", std::ios::binary);
	// Search upwards for speller.ini
	while (inif.bad() && !path.empty()) {
		path.pop_back();
		while (!path.empty() && path.back() != '/' && path.back() != '\\') {
			path.pop_back();
		}
		inif.clear();
		inif.open(path + "speller.ini", std::ios::binary);
	}

	while (std::getline(inif, line)) {
		line = trim(line);
		if (line.empty() || line[0] == '#') {
			continue;
		}

		size_t eqpos = line.find('=');
		std::string key = trim(line.substr(0, eqpos));
		std::string value = trim(line.substr(eqpos + 1));

		conf[key] = value;
		p(key, value);
	}

	if (conf.empty() || !conf.count("ENGINE")) {
		p("conf was empty");
		return false;
	}

	return true;
}

inline void w2n(const std::wstring& wide, std::string& narrow) {
	narrow.clear();
#ifdef _WIN32
	utf8::utf16to8(wide.begin(), wide.end(), std::back_inserter(narrow));
#else
	utf8::utf32to8(wide.begin(), wide.end(), std::back_inserter(narrow));
#endif
}

inline void n2w(const std::string& narrow, std::wstring& wide) {
	wide.clear();
#ifdef _WIN32
	utf8::utf8to16(narrow.begin(), narrow.end(), std::back_inserter(wide));
#else
	utf8::utf8to32(narrow.begin(), narrow.end(), std::back_inserter(wide));
#endif
}

#endif
