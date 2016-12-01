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
#ifndef ENUMSTRING_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define ENUMSTRING_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include <vector>
#include <string>
#include <algorithm>
#include <objidl.h>
#include "DLL.hpp"
#include <debugp.hpp>

template<typename String>
inline void CoCopyWString(const String& in, PWSTR* out) {
	debugp p(__FUNCTION__);
	p(in, static_cast<void*>(*out));
	*out = reinterpret_cast<LPWSTR>(CoTaskMemAlloc(sizeof(wchar_t)*(in.size() + 1)));
	std::copy(in.begin(), in.end(), *out);
	(*out)[in.size()] = 0;
	p(std::wstring(*out), static_cast<void*>(*out));
}

class EnumString : public IEnumString {
public:
	EnumString() {
		debugp p(__FUNCTION__);
		p(strings.size());
	}

	EnumString(std::wstring word) {
		debugp p(__FUNCTION__);
		strings.emplace_back(std::move(word));
		p(strings.size());
	}

	EnumString(std::vector<std::wstring> strings_)
		: strings(std::move(strings_))
	{
		debugp p(__FUNCTION__);
		p(strings.size());
	}

	STDMETHODIMP_(ULONG) AddRef() {
		debugp p(__FUNCTION__);
		return InterlockedIncrement(&refcount);
	}

	STDMETHODIMP_(ULONG) Release() {
		debugp p(__FUNCTION__);
		if (InterlockedDecrement(&refcount) == 0) {
			p("Cleanup", __LINE__);
			com_delete(this);
			return 0;
		}

		return refcount;
	}

	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppvObject) {
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

	IFACEMETHODIMP Next(ULONG celt, LPOLESTR *rgelt, _Out_ ULONG *pceltFetched) {
		debugp p(__FUNCTION__);
		p(celt);
		HRESULT hr = S_FALSE;

		ULONG i = 0;
		for (; i < celt && current < strings.size(); ++i, ++current) {
			p(i, current);
			CoCopyWString(strings[current], rgelt+i);
			p(static_cast<void*>(rgelt + i), static_cast<void*>(rgelt[i]));
		}

		if (pceltFetched) {
			*pceltFetched = i;
		}
		if (i == celt) {
			hr = S_OK;
		}

		return hr;
	}

	IFACEMETHODIMP Skip(ULONG celt) {
		debugp p(__FUNCTION__);
		p(celt);
		current += celt;

		if (current >= strings.size()) {
			current = 0;
		}

		return S_OK;
	}

	IFACEMETHODIMP Reset() {
		debugp p(__FUNCTION__);
		current = 0;
		return S_OK;
	}

	IFACEMETHODIMP Clone(IEnumString **ppenum) {
		debugp p(__FUNCTION__);
		EnumString* pnew = com_new<EnumString>(strings);
		pnew->AddRef();
		*ppenum = pnew;

		return S_OK;
	}

private:
	std::vector<std::wstring> strings;
	ULONG current = 0;
	ULONG refcount = 1;
};

class SpellingError : public ISpellingError {
public:
	SpellingError(size_t b, size_t l)
		: b(static_cast<ULONG>(b))
		, l(static_cast<ULONG>(l))
	{
		debugp p(__FUNCTION__);
		p(b, l);
	}

	STDMETHODIMP_(ULONG) AddRef() {
		debugp p(__FUNCTION__);
		return InterlockedIncrement(&refcount);
	}

	STDMETHODIMP_(ULONG) Release() {
		debugp p(__FUNCTION__);
		if (InterlockedDecrement(&refcount) == 0) {
			p("Cleanup", __LINE__);
			com_delete(this);
			return 0;
		}

		return refcount;
	}

	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppvObject) {
		//debugp p(__FUNCTION__);
		if (ppvObject == nullptr) {
			return E_POINTER;
		}

		HRESULT hr = E_NOINTERFACE;
		*ppvObject = nullptr;

		if (riid == IID_IUnknown || riid == IID_ISpellingError) {
			*ppvObject = this;
			AddRef();
			hr = S_OK;
		}

		return hr;
	}

	IFACEMETHODIMP get_CorrectiveAction(_Out_ CORRECTIVE_ACTION *value) {
		debugp p(__FUNCTION__);
		*value = CORRECTIVE_ACTION_GET_SUGGESTIONS;
		p(*value);
		return S_OK;
	}

	IFACEMETHODIMP get_Length(_Out_ ULONG *value) {
		debugp p(__FUNCTION__);
		*value = l;
		return S_OK;
	}

	IFACEMETHODIMP get_Replacement(_Out_ LPWSTR *value) {
		debugp p(__FUNCTION__);
		*value = nullptr;
		return S_OK;
	}

	IFACEMETHODIMP get_StartIndex(_Out_ ULONG *value) {
		debugp p(__FUNCTION__);
		*value = b;
		return S_OK;
	}

private:
	ULONG b, l;
	ULONG refcount = 1;
};

class EnumSpellingError : public IEnumSpellingError {
public:
	EnumSpellingError(std::vector<SpellingError> errors_)
		: errors(std::move(errors_))
	{
		debugp p(__FUNCTION__);
		p(errors.size());
	}

	STDMETHODIMP_(ULONG) AddRef() {
		debugp p(__FUNCTION__);
		return InterlockedIncrement(&refcount);
	}

	STDMETHODIMP_(ULONG) Release() {
		debugp p(__FUNCTION__);
		if (InterlockedDecrement(&refcount) == 0) {
			p("Cleanup", __LINE__);
			com_delete(this);
			return 0;
		}

		return refcount;
	}

	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppvObject) {
		//debugp p(__FUNCTION__);
		if (ppvObject == nullptr) {
			return E_POINTER;
		}

		HRESULT hr = E_NOINTERFACE;
		*ppvObject = nullptr;

		if (riid == IID_IUnknown || riid == IID_IEnumSpellingError) {
			*ppvObject = this;
			AddRef();
			hr = S_OK;
		}

		return hr;
	}

	IFACEMETHODIMP Next(ISpellingError **value) {
		debugp p(__FUNCTION__);
		if (i >= errors.size()) {
			return S_FALSE;
		}

		*value = com_new<SpellingError>(errors[i]);

		++i;
		return S_OK;
	}

private:
	std::vector<SpellingError> errors;
	ULONG i = 0;
	ULONG refcount = 1;
};

#endif
