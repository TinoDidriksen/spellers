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

#include <vector>
#include <set>
#include <string>
#include <cstdio>
#include <map>
#include <cctype>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <shared.hpp>

#ifdef _WIN32
	#include <windows.h>

	#ifdef SHIM_EXPORTS
		#define SPELLER_API __declspec(dllexport)
	#else
		#define SPELLER_API __declspec(dllimport)
	#endif
#else
	#include <dlfcn.h>
	#include <unistd.h>
	#include <popen_plus.h>

	#ifdef SHIM_EXPORTS
		#define SPELLER_API __attribute__ ((visibility ("default")))
	#else
		#define SPELLER_API
	#endif
#endif

#include <debugp.hpp>
#ifndef NDEBUG
std::ofstream debug("C:/Temp/Tino/debug-speller.txt");
thread_local size_t debugd = 0;
#endif

// Unnamed namespace to hide all these globals
namespace {
#ifdef _WIN32
HANDLE g_hChildStd_IN_Rd = 0;
HANDLE g_hChildStd_IN_Wr = 0;
HANDLE g_hChildStd_OUT_Rd = 0;
HANDLE g_hChildStd_OUT_Wr = 0;
#else
popen_plus_process *child = 0;
#endif

std::map<std::string, std::string> conf;
std::unordered_set<std::string> valid_words;
std::unordered_map<std::string, std::vector<std::string>> invalid_words;
std::vector<const char*> rv_alts;
std::string cbuffer;

#ifdef _WIN32
void showLastError(const std::string& err) {
	std::string msg = conf["NAME"] + " error location: ";
	msg += err;
	msg += "\n\n";
	char *fmt = 0;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, GetLastError(), MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), (LPSTR)&fmt, 0, 0);
	msg += "GetLastError: ";
	msg += fmt;
	msg += '\n';
	LocalFree(fmt);
	MessageBoxA(0, msg.c_str(), "Speller Error", MB_OK|MB_ICONERROR);
}
#else
std::string formatLastError(std::string msg = "") {
	if (!msg.empty()) {
		msg += ' ';
	}
	msg += "strerror: ";
	msg += strerror(errno);
	return msg;
}
#endif

bool checkValidWord(const std::string& word, size_t suggs = 0) {
	debugp p(__FUNCTION__);
	p(word);

	if (valid_words.find(word) != valid_words.end()) {
		return true;
	}
	if (invalid_words.find(word) != invalid_words.end()) {
		return false;
	}

	cbuffer.resize(std::numeric_limits<size_t>::digits10+2);
	cbuffer.resize(sprintf(&cbuffer[0], "%llu ", static_cast<unsigned long long>(suggs)));
	cbuffer += word;
	cbuffer += '\n';

#ifdef _WIN32
	DWORD bytes = 0, bytes_read = 0;
	if (!WriteFile(g_hChildStd_IN_Wr, cbuffer.c_str(), static_cast<DWORD>(cbuffer.size()), &bytes, 0) || bytes != cbuffer.size()) {
		showLastError("checkValidWord WriteFile");
		return false;
	}
	cbuffer.resize(1);
	if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[0], 1, &bytes_read, 0) || bytes_read != 1) {
		showLastError("checkValidWord ReadFile 1");
		return false;
	}
	if (!PeekNamedPipe(g_hChildStd_OUT_Rd, 0, 0, 0, &bytes, 0)) {
		showLastError("checkValidWord PeekNamedPipe");
		return false;
	}
	if (bytes) {
		cbuffer.resize(1+bytes);
		if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[1], bytes, &bytes_read, 0) || bytes != bytes_read) {
			showLastError("checkValidWord ReadFile 2");
			return false;
		}
	}
#else
	if (fwrite(cbuffer.c_str(), 1, cbuffer.size(), child->write_fp) != cbuffer.size()) {
		std::string msg = formatLastError("Process.write(char*,size_t)");
		throw std::runtime_error(msg);
	}
	fflush(child->write_fp);
	cbuffer.resize(0);
	cbuffer.resize(2048);
	for (size_t i=0, off=0 ; (fread(&cbuffer[off], 1, cbuffer.size()-off, child->read_fp) != cbuffer.size()-off) && i<10 ; ++i, off = cbuffer.size()) {
		std::string msg = formatLastError("Process.read(char*,size_t)");
		throw std::runtime_error(msg);
	}
#endif

	cbuffer = trim(cbuffer);

	if (cbuffer[0] == '*') {
		valid_words.insert(word);
		return true;
	}

	if (suggs == 0) {
		return false;
	}

	std::vector<std::string>& alts = invalid_words[word];
	alts.clear();

	// This includes # for no alternatives and ! for error
	if (cbuffer[0] != '&') {
		return false;
	}

	size_t e = cbuffer.find("\t");
	if (e == std::string::npos) {
		return false;
	}

	size_t b = e + 1;
	while ((e = cbuffer.find("\t", b)) != std::string::npos) {
		alts.emplace_back(cbuffer.begin()+b, cbuffer.begin()+e);
		b = e + 1;
	}
	alts.emplace_back(cbuffer.begin() + b, cbuffer.end());

	return false;
}
}

extern "C" int SPELLER_API shim_init() {
	debugp p(__FUNCTION__);

#ifdef _WIN32
	SECURITY_ATTRIBUTES saAttr = {sizeof(saAttr), 0, true};

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
		showLastError("shim_init CreatePipe 1");
		return -__LINE__;
	}
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
		showLastError("shim_init SetHandleInformation 1");
		return -__LINE__;
	}
	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) {
		showLastError("shim_init CreatePipe 2");
		return -__LINE__;
	}
	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
		showLastError("shim_init SetHandleInformation 2");
		return -__LINE__;
	}

	PROCESS_INFORMATION piProcInfo = {0};
	STARTUPINFOA siStartInfo = {sizeof(siStartInfo)};

	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
#endif

	if (!read_conf(conf)) {
		return -__LINE__;
	}

	std::string path = conf["PATH"];
	path.append("backend");

	std::string cmdline(path);
	cmdline += '/';
	cmdline.append(conf["ENGINE"]);
	cmdline.append(1, 0);

#ifdef _WIN32
	BOOL bSuccess = CreateProcessA(0,
		&cmdline[0],
		0,
		0,
		TRUE,
		CREATE_NO_WINDOW | BELOW_NORMAL_PRIORITY_CLASS,
		0,
		path.c_str(),
		&siStartInfo,
		&piProcInfo);

	if (!bSuccess) {
		std::string msg = conf["NAME"] + " could not start " + conf["ENGINE"] + "!\n\n";
		char *fmt = 0;
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, GetLastError(), MAKELANGID(LANG_NEUTRAL,SUBLANG_NEUTRAL), (LPSTR)&fmt, 0, 0);
		msg += "GetLastError: ";
		msg += fmt;
		msg += '\n';
		LocalFree(fmt);
		MessageBoxA(0, msg.c_str(), "Speller launch error", MB_OK|MB_ICONERROR);
		return -__LINE__;
	}
	else {
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}

	DWORD bytes = 0, bytes_read = 0;
	cbuffer.resize(4);
	if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[0], 4, &bytes_read, 0) || bytes_read != 4) {
		showLastError("shim_init ReadFile 1");
		return -__LINE__;
	}
	if (!PeekNamedPipe(g_hChildStd_OUT_Rd, 0, 0, 0, &bytes, 0)) {
		showLastError("shim_init PeekNamedPipe");
		return -__LINE__;
	}
	if (bytes) {
		cbuffer.resize(4 + bytes);
		if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[4], bytes, &bytes_read, 0) || bytes != bytes_read) {
			showLastError("shim_init ReadFile 2");
		return -__LINE__;
		}
	}
#else
	child = popen_plus(cmdline.c_str());
	if (child == 0) {
		std::string msg = "Process could not start!\nCmdline: ";
		msg += cmdline.c_str();
		msg += '\n';
		msg = formatLastError(msg);
		throw std::runtime_error(msg);
	}

	cbuffer.resize(31);
	if (fread(&cbuffer[0], 1, cbuffer.size(), child->read_fp) != cbuffer.size()) {
		std::string msg = formatLastError("Process.read(char*,size_t)");
		throw std::runtime_error(msg);
	}
#endif

	cbuffer = trim(cbuffer);
	if (cbuffer != "@@ hfst-ospell-office is alive") {
		return -__LINE__;
	}

	return 0;
}

extern "C" void SPELLER_API shim_terminate() {
	debugp p(__FUNCTION__);

#ifdef _WIN32
	CloseHandle(g_hChildStd_IN_Rd);
	CloseHandle(g_hChildStd_IN_Wr);
	CloseHandle(g_hChildStd_OUT_Rd);
	CloseHandle(g_hChildStd_OUT_Wr);
#else
	popen_plus_kill(child);
	popen_plus_close(child);
#endif
}

extern "C" int SPELLER_API shim_is_valid_word(const char *word) {
	debugp p(__FUNCTION__);
	std::string str(word);
	p(str);
	return static_cast<int>(checkValidWord(str));
}

typedef const char** ccharpp_t;
extern "C" ccharpp_t SPELLER_API shim_find_alternatives(const char *word, int suggs) {
	debugp p(__FUNCTION__);
	std::string str(word);
	p(str);

	if (checkValidWord(str, suggs)) {
		return 0;
	}

	rv_alts.clear();
	const std::vector<std::string>& alts = invalid_words[str];
	rv_alts.reserve(alts.size() + 1);

	for (size_t i = 0; i < alts.size(); ++i) {
		rv_alts.push_back(alts[i].c_str());
	}
	rv_alts.push_back(0);

	return &rv_alts[0];
}
