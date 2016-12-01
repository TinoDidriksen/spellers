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

#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <memory>
#include <cstdint>
#include "DLL.hpp"
#include "ClassFactory.hpp"
#include <windows.h>
#include <shared.hpp>
#include <debugp.hpp>

#ifndef NDEBUG
std::ofstream debug("C:/Temp/Tino/debug-speller-win8.txt");
thread_local size_t debugd = 0;
#endif

std::map<std::string, std::string> conf;
std::vector<std::wstring> locales;
GUID IID_Guid = {};
std::unique_ptr<ClassFactory> factory;
size_t refs = 0;
size_t locks = 0;

bool read_conf() {
	debugp p(__FUNCTION__);

	if (!read_conf(conf)) {
		return false;
	}

	IID_Guid = String_to_UUID(conf["UUID"]);

	std::istringstream ss(conf["LOCALES"]);
	std::string locale;
	while (std::getline(ss, locale, ' ')) {
		locales.emplace_back(locale.begin(), locale.end());
	}

	return true;
}

STDAPI __declspec(dllexport) DllGetClassObject(REFCLSID objGuid, REFIID factoryGuid, void **factoryHandle) {
	debugp p(__FUNCTION__);
	p(UUID_to_String(objGuid));
	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
	*factoryHandle = nullptr;

	if (objGuid == IID_Guid) {
		if (!factory) {
			factory = std::make_unique<ClassFactory>();
		}
		hr = factory->QueryInterface(factoryGuid, factoryHandle);
	}

	return hr;
}

STDAPI __declspec(dllexport) DllCanUnloadNow() {
	debugp p(__FUNCTION__);
	return (refs || locks) ? S_FALSE : S_OK;
}

BOOL WINAPI __declspec(dllexport) DllMain(HINSTANCE instance, DWORD fdwReason, LPVOID lpvReserved) {
	debugp p(__FUNCTION__);
	p(fdwReason);
	(void)instance;
	(void)lpvReserved;

	switch (fdwReason) {
	case DLL_PROCESS_ATTACH: {
		refs = locks = 0;

		read_conf();

		//DisableThreadLibraryCalls(instance);
		break;
	}
	}

	return 1;
}
