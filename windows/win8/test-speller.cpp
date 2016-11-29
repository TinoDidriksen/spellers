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

#include "DLL.hpp"
#include "Speller.hpp"
bool read_conf();

int main() {
	read_conf();

	Speller speller(locales.front());

	IEnumSpellingError *ese;
	speller.Check(L"illlu", &ese);

	ISpellingError *se;
	ese->Next(&se);

	IEnumString *es;
	speller.Suggest(L"illlu", &es);

	LPOLESTR str;
	while (es->Next(1, &str, nullptr) == S_OK) {
		std::wcout << str << std::endl;
		CoTaskMemFree(str);
	}

	es->Release();
	se->Release();
	ese->Release();
}
