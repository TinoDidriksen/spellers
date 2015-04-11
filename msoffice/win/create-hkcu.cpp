#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <map>
#include <stdint.h>
#include <windows.h>

// Not allowed to take the address of main, so a function that we can take the address of
std::wstring get_path() {
	MEMORY_BASIC_INFORMATION mbiInfo = { 0 };
	std::wstring path(MAX_PATH + 1, 0);
	if (VirtualQuery((void*)get_path, &mbiInfo, sizeof(mbiInfo))) {
		GetModuleFileName((HMODULE)(mbiInfo.AllocationBase), &path[0], MAX_PATH);
	}
	if (path[0] == 0) {
		GetModuleFileName(GetModuleHandle(L"create-hkcu.exe"), &path[0], MAX_PATH);
	}
	while (!path.empty() && path.back() != '/' && path.back() != '\\') {
		path.pop_back();
	}
	return path;
}

std::string trim(std::string str) {
	while (!str.empty() && std::isspace(str[str.size() - 1])) {
		str.resize(str.size() - 1);
	}
	while (!str.empty() && std::isspace(str[0])) {
		str.erase(str.begin());
	}
	return str;
}

int main() {
	std::map<std::string, std::string> conf;

	std::wstring wpath = get_path();
	std::string path(wpath.begin(), wpath.end());

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

	if (conf.empty() || !conf.count("LOCALE")) {
		return -1;
	}

	std::string buf8("Software\\Microsoft\\Shared Tools\\Proofing Tools\\1.0\\Override\\");
	buf8.append(conf["LOCALE"]);

	HKEY key;
	LONG rv = 0;

	if ((rv = RegCreateKeyExA(HKEY_CURRENT_USER, buf8.c_str(), 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key, 0)) != ERROR_SUCCESS) {
		std::cerr << "Error: Could not create " << buf8 << ": " << rv << std::endl;
		return -rv;
	}

	std::string data;
	if (data.assign(path + "office32.dll").empty()
		|| (rv = RegSetValueExA(key, "DLL", 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), (data.size()+1)*sizeof(data[0]))) != ERROR_SUCCESS) {
		std::cerr << "Error: Could not write DLL: " << rv << std::endl;
	}
	if (data.assign(path + "office64.dll").empty()
		|| (rv = RegSetValueExA(key, "DLL64", 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), (data.size()+1)*sizeof(data[0]))) != ERROR_SUCCESS) {
		std::cerr << "Error: Could not write DLL64: " << rv << std::endl;
	}
	if (data.assign(path + "speller.ini").empty()
		|| (rv = RegSetValueExA(key, "LEX", 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), (data.size()+1)*sizeof(data[0]))) != ERROR_SUCCESS
		|| (rv = RegSetValueExA(key, "LEX64", 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), (data.size()+1)*sizeof(data[0]))) != ERROR_SUCCESS) {
		std::cerr << "Error: Could not write LEX and/or LEX64: " << rv << std::endl;
	}

	RegCloseKey(key);
}
