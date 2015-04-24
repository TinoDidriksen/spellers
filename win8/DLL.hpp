/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#pragma once
#ifndef DLL_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define DLL_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include <string>
#include <map>
#include <vector>
#include <cctype>
#include <guiddef.h>

enum {
	GUID_SpellerFactory,
	GUID_Speller,
	NUM_GUIDS,
};

extern std::map<std::string, std::string> conf;
extern std::vector<std::wstring> locales;
extern GUID IID_Guid;
extern size_t refs;
extern size_t locks;

inline std::string UUID_to_String(REFIID guid) {
	std::string uuid(36, 0);
	sprintf(&uuid[0], "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return uuid;
}

inline std::string trim(std::string str) {
	while (!str.empty() && std::isspace(str[str.size() - 1])) {
		str.resize(str.size() - 1);
	}
	while (!str.empty() && std::isspace(str[0])) {
		str.erase(str.begin());
	}
	return str;
}

#endif
