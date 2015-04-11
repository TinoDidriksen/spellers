#include <iostream>
#include <string>
#include <stdint.h>

#include <winsdkver.h>
#include <SDKDDKVer.h>
#include <windows.h>

int main() {
	std::string buf(1024, 0);
	uint32_t len = buf.size();
	HKEY key;
	LONG rv = 0;

	if ((rv = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Active Setup\\Installed Components\\{A3AC5A53-231B-44A2-A28B-E59CCA979FB0}", 0, KEY_READ|KEY_WOW64_32KEY, &key)) != ERROR_SUCCESS
		&& (rv = RegOpenKeyExA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Active Setup\\Installed Components\\{A3AC5A53-231B-44A2-A28B-E59CCA979FB0}", 0, KEY_READ|KEY_WOW64_64KEY, &key)) != ERROR_SUCCESS) {
		std::cerr << "Error: Could not open HKLM\\Software\\Microsoft\\Active Setup\\Installed Components\\{A3AC5A53-231B-44A2-A28B-E59CCA979FB0} for reading: " << rv << std::endl;
		return -rv;
	}
	if ((rv = RegQueryValueExA(key, "StubPath", 0, 0, reinterpret_cast<LPBYTE>(&buf[0]), reinterpret_cast<LPDWORD>(&len))) != ERROR_SUCCESS) {
		std::cerr << "Error: Could not read StubPath: " << rv << std::endl;
		return -rv;
	}
	RegCloseKey(key);

	if ((rv = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Shared Tools\\Proofing Tools\\1.0\\Override\\kl-GL", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &key, 0)) != ERROR_SUCCESS) {
		std::cerr << "Error: Could not create HKCU\\Software\\Microsoft\\Shared Tools\\Proofing Tools\\1.0\\Override\\kl-GL: " << rv << std::endl;
		return -rv;
	}

	// Strip create_hkcu.exe\0 from the string
	buf.resize(len-1-15);

	std::string data;
	if (data.assign(buf + "kalspell.dll").empty() || (rv = RegSetValueExA(key, "DLL", 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), data.size()+1)) != ERROR_SUCCESS) {
		std::cerr << "Error: Could not write DLL: " << rv << std::endl;
	}
	if (data.assign(buf + "kalspell64.dll").empty() || (rv = RegSetValueExA(key, "DLL64", 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), data.size()+1)) != ERROR_SUCCESS) {
		std::cerr << "Error: Could not write DLL64: " << rv << std::endl;
	}
	if (data.assign(buf + "kal.foma").empty() || (rv = RegSetValueExA(key, "LEX", 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), data.size()+1)) != ERROR_SUCCESS
		 || (rv = RegSetValueExA(key, "LEX64", 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()), data.size()+1)) != ERROR_SUCCESS) {
		std::cerr << "Error: Could not write LEX and/or LEX64: " << rv << std::endl;
	}

	RegCloseKey(key);
}
