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

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include "windows.h"

extern const char binary_32_msi_start[];
extern const char binary_32_msi_end[];
extern const char binary_64_msi_start[];
extern const char binary_64_msi_end[];

void install(const char *path, char *fname, const char *start, const char *end) {
	remove(fname);
	strcat(fname, ".msi");

	size_t sz = end - start;
	std::cout << "Writing " << fname << " (" << sz << " bytes)" << std::endl;
	std::ofstream out(fname, std::ios::binary);
	out.write(start, sz);
	out.close();

	std::cout << "Executing " << fname << std::endl;
	SHELLEXECUTEINFO sei = {sizeof(sei)};
	sei.fMask = SEE_MASK_NOCLOSEPROCESS;
	sei.hwnd = NULL;
	sei.lpVerb = NULL;
	sei.lpFile = fname;
	sei.lpParameters = NULL;
	sei.lpDirectory = path;
	sei.nShow = SW_SHOW;
	sei.hInstApp = NULL;
	ShellExecuteEx(&sei);
	WaitForSingleObject(sei.hProcess, INFINITE);
	CloseHandle(sei.hProcess);

	std::cout << "Cleaning up" << std::endl;
	remove(fname);
}

int main() {
	char path[MAX_PATH]{};
	GetTempPathA(MAX_PATH, path);
	char fname[MAX_PATH]{};

	std::cout << "Installing " << SPELLER_NAME << std::endl;

	SYSTEM_INFO si;
	GetNativeSystemInfo(&si);
	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
		std::cout << "64 bit detected..." << std::endl;
		GetTempFileName(path, "x64", 0, fname);
		install(path, fname, binary_64_msi_start, binary_64_msi_end);
	}

	GetTempFileName(path, "x32", 0, fname);
	install(path, fname, binary_32_msi_start, binary_32_msi_end);
}
