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

#include "SpellerFactory.hpp"
#include "DLL.hpp"
#include "EnumString.hpp"
#include <COM.hpp>
#include <debugp.hpp>

const IID IID_ISpellCheckProviderFactory = { 0x9F671E11, 0x77D6, 0x4C92, { 0xAE, 0xFB, 0x61, 0x52, 0x15, 0xE3, 0xA4, 0xBE } };

SpellerFactory::~SpellerFactory() {
	for (auto& it : spellers) {
		it.second->Release();
	}
}

HRESULT STDMETHODCALLTYPE SpellerFactory::QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject) {
	debugp p(__FUNCTION__);
	p(UUID_to_String(riid));
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
	*ppvObject = nullptr;

	if (riid == IID_IUnknown || riid == IID_ISpellCheckProviderFactory || riid == IID_Guid) {
		*ppvObject = this;
		hr = S_OK;
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE SpellerFactory::AddRef() {
	debugp p(__FUNCTION__);
	InterlockedIncrement(&refcount);
	p(refcount);
	return refcount;
}

ULONG STDMETHODCALLTYPE SpellerFactory::Release() {
	debugp p(__FUNCTION__);

	if (InterlockedDecrement(&refcount) == 0) {
		p(__LINE__);
		com_delete(this);
		return 0;
	}

	return refcount;
}

IFACEMETHODIMP SpellerFactory::get_SupportedLanguages(_COM_Outptr_ IEnumString** value) {
	debugp p(__FUNCTION__);
	if (value == nullptr) {
		return E_POINTER;
	}
	*value = com_new<EnumString>(locales);
	return S_OK;
}

IFACEMETHODIMP SpellerFactory::IsSupported(_In_ PCWSTR languageTag, _Out_ BOOL* value) {
	debugp p(__FUNCTION__);
	if (value == nullptr) {
		return E_POINTER;
	}
	*value = false;

	for (auto& locale : locales) {
		if (CompareStringOrdinal(languageTag, -1, locale.c_str(), -1, true) == CSTR_EQUAL) {
			*value = true;
			break;
		}
	}

	return S_OK;
}

IFACEMETHODIMP SpellerFactory::CreateSpellCheckProvider(_In_ PCWSTR languageTag, _COM_Outptr_ ISpellCheckProvider** value) {
	debugp p(__FUNCTION__);
	if (value == nullptr) {
		return E_POINTER;
	}
	BOOL isSupported = false;
	HRESULT hr = IsSupported(languageTag, &isSupported);
	if (SUCCEEDED(hr) && !isSupported) {
		hr = E_INVALIDARG;
	}

	std::wstring locale(languageTag);
	if (!spellers[locale]) {
		spellers[locale] = com_new<Speller>(languageTag);
		spellers[locale]->AddRef();
	}
	*value = spellers[locale];

	return hr;
}
