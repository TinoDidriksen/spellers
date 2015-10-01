/*
* Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
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

#include <fstream>
#include <string>
#include <map>
#include <cctype>
#include <debugp.hpp>

#ifdef _WIN32
	#include <windows.h>
#else
	#define _GNU_SOURCE
	#include <dlfcn.h>
	#include <cstdio>
#endif

inline std::string trim(std::string str) {
	while (!str.empty() && (str.back() == 0 || std::isspace(str.back()))) {
		str.resize(str.size() - 1);
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
	if (VirtualQuery(read_conf, &mbiInfo, sizeof(mbiInfo))) {
		GetModuleFileNameA((HMODULE)(mbiInfo.AllocationBase), &path[0], MAX_PATH);
	}
#else
	Dl_info dl_info = {};
	dladdr((void*)read_conf, &dl_info);
	path.assign(dl_info.dli_fname);
#endif
	if (path[0] == 0) {
		p("path was empty");
		return false;
	}
	while (!path.empty() && path.back() != '/' && path.back() != '\\') {
		path.pop_back();
	}

	std::string line;
	std::ifstream inif(path + "speller.ini", std::ios::binary);
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

	conf["PATH"] = path;
	conf["SERVICE_NAME"] = conf["NAME"] + " Speller Service";

	// Create a pipe name from the locale prefix
	conf["PIPE"] = "\\\\.\\pipe\\speller_";
	conf["PIPE"] += conf["LOCALES"].substr(0, conf["LOCALES"].find('-'));

	return true;
}

#endif
