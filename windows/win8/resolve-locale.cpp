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

#include <algorithm>
#include <cstring>
#include <iostream>
#include "windows.h"

int main(int argc, char *argv[]) {
	wchar_t in[256]{};
	wchar_t out[256]{};

	std::copy(argv[1], argv[1] + strlen(argv[1]), in);

	if (ResolveLocaleName(in, out, 256) == 0) {
		std::wcout << "Resolving " << argv[1] << " yielded error " << GetLastError() << std::endl;
		return 1;
	}

	std::wcout << in << " = " << out << std::endl;
}
