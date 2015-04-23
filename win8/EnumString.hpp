/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#pragma once
#ifndef ENUMSTRING_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define ENUMSTRING_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include <vector>
#include <string>
#include <algorithm>
#include <ObjIdl.h>

template<typename String>
inline void CoCopyWString(const String& in, PWSTR* out) {
	*out = reinterpret_cast<LPWSTR>(CoTaskMemAlloc(sizeof(wchar_t)*(in.size() + 1)));
	std::copy(in.begin(), in.end(), *out);
}

class EnumString : public IEnumString {
public:
	EnumString() {
	}

	EnumString(std::vector<std::wstring> strings) :
		strings(strings)
	{
	}

	STDMETHODIMP_(ULONG) AddRef() {
		return InterlockedIncrement(&refcount);
	}

	STDMETHODIMP_(ULONG) Release() {
		if (InterlockedDecrement(&refcount) == 0) {
			delete this;
			return 0;
		}

		return refcount;
	}

	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObject) {
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
		current += celt;

		if (current >= strings.size()) {
			current = 0;
		}

		return S_OK;
	}

	STDMETHODIMP Reset() {
		current = 0;
		return S_OK;
	}

	STDMETHODIMP Clone(IEnumString **ppenum) {
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