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

#pragma once
#ifndef COM_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define COM_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include <string>
#include <algorithm>
#include <memory>
#include <cstdint>
#include <cstdlib>
#include <guiddef.h>
#include <objbase.h>

template <typename T>
inline T identity(T);

template<typename T, typename... Args>
inline T* com_new(Args&&... args) {
	char *buf = reinterpret_cast<char*>(CoTaskMemAlloc(sizeof(T)));
	T *t = new (buf) T(std::forward<Args>(args)...);
	return t;
}

template<typename T>
inline void com_delete(T * t) {
	t->~T();
	CoTaskMemFree(t);
}

template<typename T>
inline void com_release(T * t) {
	t->Release();
}

template<typename T, typename... Args>
inline std::unique_ptr<T, decltype(identity(&com_release<T>))> com_make_unique(Args&&... args) {
	T *t = com_new<T>(std::forward<Args>(args)...);
	std::unique_ptr<T, decltype(identity(&com_release<T>))> u(t, com_release<T>);
	return u;
}

inline GUID String_to_UUID(std::string str, uint8_t add = 0) {
	GUID uuid;

	uuid.Data1 = strtoul(str.c_str(), 0, 16);
	uuid.Data2 = static_cast<uint16_t>(strtoul(str.c_str() + 9, 0, 16));
	uuid.Data3 = static_cast<uint16_t>(strtoul(str.c_str() + 14, 0, 16));

	str.erase(str.begin(), str.begin() + 19);
	std::remove(str.begin(), str.end(), '-');
	for (size_t i = 0; i < sizeof(uuid.Data4); ++i) {
		char bs[] = { str[i * 2], str[i * 2 + 1], 0 };
		uuid.Data4[i] = static_cast<uint8_t>(strtoul(bs, 0, 16));
	}

	uuid.Data4[7] += add;

	return uuid;
}

inline std::string UUID_to_String(REFIID guid) {
	std::string uuid(36, 0);
	sprintf(&uuid[0], "%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
	return uuid;
}

inline GUID Offset_UUID(GUID uuid, uint8_t add = 0) {
	uuid.Data4[7] = (uuid.Data4[7] & 0xF0) + add;
	return uuid;
}

#endif
