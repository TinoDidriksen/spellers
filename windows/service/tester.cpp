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

#include <shared.hpp>
#include <debugp.hpp>
#include <shellapi.h>
#include <cwchar>

#ifndef NDEBUG
std::ofstream debug("C:/Temp/Tino/debug-speller-tester.txt");
thread_local size_t debugd = 0;
#endif

int main() {
	debugp p(__FUNCTION__);
	std::map<std::string, std::string> conf;

	read_conf(conf);

	std::string cbuffer = "7 illlu\n";

	HANDLE pipe = CreateFileA(conf["PIPE"].c_str(), GENERIC_READ | FILE_WRITE_DATA, 0, nullptr, OPEN_EXISTING, 0, nullptr);

	if (pipe == INVALID_HANDLE_VALUE) {
		p("CreateFileA failed", GetLastError());
		return GetLastError();
	}

	p("Writing pipe", cbuffer);
	DWORD bytes = 0;
	DWORD bytes_read = 0;
	if (!WriteFile(pipe, cbuffer.c_str(), cbuffer.size(), &bytes, 0) || bytes != cbuffer.size()) {
		p("WriteFile(pipe)", GetLastError());
		return GetLastError();
	}

	p("Reading pipe");
	bytes = 0;
	bytes_read = 0;
	cbuffer.resize(1);
	if (!ReadFile(pipe, &cbuffer[0], 1, &bytes_read, 0) || bytes_read != 1) {
		p("ReadFile(pipe) 1", GetLastError());
		return GetLastError();
	}
	if (!PeekNamedPipe(pipe, 0, 0, 0, &bytes, 0)) {
		p("PeekNamedPipe(pipe)", GetLastError());
		return GetLastError();
	}
	if (bytes) {
		cbuffer.resize(1 + bytes);
		if (!ReadFile(pipe, &cbuffer[1], bytes, &bytes_read, 0) || bytes != bytes_read) {
			p("ReadFile(pipe) 2", GetLastError());
			return GetLastError();
		}
	}

	p(cbuffer);

	CloseHandle(pipe);
}
