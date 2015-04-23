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

#ifdef DLL_EXPORTS
	#define SPELLER_API __declspec(dllexport)
#else
	#define SPELLER_API __declspec(dllimport)
#endif

enum {
	GUID_SpellerFactory,
	GUID_Speller,
	NUM_GUIDS,
};

extern std::map<std::string, std::string> conf;
extern std::vector<std::wstring> locales;
extern GUID IID_Guids[NUM_GUIDS];
extern size_t refs;
extern size_t locks;

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
