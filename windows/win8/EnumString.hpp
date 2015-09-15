/*
* Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
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
#ifndef ENUMSTRING_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define ENUMSTRING_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include <vector>
#include <string>
#include <algorithm>
#include <ObjIdl.h>
#include "DLL.hpp"
#include <debugp.hpp>

template<typename String>
inline void CoCopyWString(const String& in, PWSTR* out) {
	debugp p(__FUNCTION__);
	p(in);
	*out = reinterpret_cast<LPWSTR>(CoTaskMemAlloc(sizeof(wchar_t)*(in.size() + 1)));
	std::copy(in.begin(), in.end(), *out);
	(*out)[in.size()] = 0;
	p(std::wstring(*out));
}

class EnumString : public IEnumString {
public:
	EnumString() {
		debugp p(__FUNCTION__);
	}

	EnumString(std::vector<std::wstring> strings) :
		strings(strings)
	{
		debugp p(__FUNCTION__);
	}

	STDMETHODIMP_(ULONG) AddRef() {
		debugp p(__FUNCTION__);
		return InterlockedIncrement(&refcount);
	}

	STDMETHODIMP_(ULONG) Release() {
		debugp p(__FUNCTION__);
		if (InterlockedDecrement(&refcount) == 0) {
			p(__LINE__);
			com_delete(this);
			return 0;
		}

		return refcount;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) {
		//debugp p(__FUNCTION__);
		if (ppvObject == nullptr) {
			return E_POINTER;
		}

		HRESULT hr = E_NOINTERFACE;
		*ppvObject = nullptr;

		if (riid == IID_IUnknown || riid == IID_IEnumString) {
			*ppvObject = this;
			AddRef();
			hr = S_OK;
		}

		return hr;
	}

	STDMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, ULONG *pceltFetched) {
		debugp p(__FUNCTION__);
		p(celt);
		HRESULT hr = S_FALSE;

		ULONG i = 0;
		for (; i < celt && current < strings.size(); ++i, ++current) {
			CoCopyWString(strings[current], rgelt+i);
		}

		if (celt > 1) {
			*pceltFetched = i;
		}
		if (i == celt) {
			hr = S_OK;
		}

		return hr;
	}

	STDMETHODIMP Skip(ULONG celt) {
		debugp p(__FUNCTION__);
		p(celt);
		current += celt;

		if (current >= strings.size()) {
			current = 0;
		}

		return S_OK;
	}

	STDMETHODIMP Reset() {
		debugp p(__FUNCTION__);
		current = 0;
		return S_OK;
	}

	STDMETHODIMP Clone(IEnumString **ppenum) {
		debugp p(__FUNCTION__);
		EnumString* pnew = new EnumString(strings);
		pnew->AddRef();
		*ppenum = pnew;

		return S_OK;
	}

private:
	std::vector<std::wstring> strings;
	ULONG current = 0;
	ULONG refcount = 1;
};

#endif
