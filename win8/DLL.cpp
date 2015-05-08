/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
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
#include <debugp.hpp>

#ifndef NDEBUG
std::ofstream debug("C:/Temp/Tino/debug-speller.txt");
size_t debugd = 0;
#endif

std::map<std::string, std::string> conf;
std::vector<std::wstring> locales;
GUID IID_Guid = {};
std::unique_ptr<ClassFactory> factory;
size_t refs = 0;
size_t locks = 0;

bool read_conf() {
	debugp p(__FUNCTION__);
	MEMORY_BASIC_INFORMATION mbiInfo = { 0 };
	std::string path(MAX_PATH + 1, 0);
	if (VirtualQuery(read_conf, &mbiInfo, sizeof(mbiInfo))) {
		GetModuleFileNameA((HMODULE)(mbiInfo.AllocationBase), &path[0], MAX_PATH);
	}
	if (path[0] == 0) {
		#ifdef _WIN64
		GetModuleFileNameA(GetModuleHandleA("speller-win8.dll"), &path[0], MAX_PATH);
		#else
		GetModuleFileNameA(GetModuleHandleA("speller-win8.dll"), &path[0], MAX_PATH);
		#endif
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
	}

	if (conf.empty() || !conf.count("ENGINE") || !conf.count("UUID") || !conf.count("LOCALES")) {
		return false;
	}

	conf["PATH"] = path;

	p(conf["PATH"]);

	GUID uuid;
	uuid.Data1 = strtoul(conf["UUID"].c_str(), 0, 16);
	uuid.Data2 = static_cast<uint16_t>(strtoul(conf["UUID"].c_str() + 9, 0, 16));
	uuid.Data3 = static_cast<uint16_t>(strtoul(conf["UUID"].c_str() + 14, 0, 16));

	std::string bytes = conf["UUID"].substr(19);
	std::remove(bytes.begin(), bytes.end(), '-');
	for (size_t i = 0; i < sizeof(uuid.Data4); ++i) {
		char bs[] = {bytes[i*2], bytes[i*2+1], 0};
		uuid.Data4[i] = static_cast<uint8_t>(strtoul(bs, 0, 16));
	}

	IID_Guid = uuid;

	std::istringstream ss(conf["LOCALES"]);
	std::string locale;
	while (std::getline(ss, locale, ' ')) {
		locales.emplace_back(locale.begin(), locale.end());
	}

	return true;
}

STDAPI DllGetClassObject(REFCLSID objGuid, REFIID factoryGuid, void **factoryHandle) {
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

STDAPI DllCanUnloadNow() {
	debugp p(__FUNCTION__);
	return (refs || locks) ? S_FALSE : S_OK;
}

BOOL WINAPI DllMain(HINSTANCE instance, DWORD fdwReason, LPVOID lpvReserved) {
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
