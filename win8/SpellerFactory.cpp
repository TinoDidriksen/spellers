/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#include "SpellerFactory.hpp"
#include "DLL.hpp"
#include "EnumString.hpp"
#include <debugp.hpp>

const IID IID_ISpellCheckProviderFactory = { 0x9F671E11, 0x77D6, 0x4C92, { 0xAE, 0xFB, 0x61, 0x52, 0x15, 0xE3, 0xA4, 0xBE } };

HRESULT STDMETHODCALLTYPE SpellerFactory::QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject) {
	debugp p(__FUNCTION__);
	p(UUID_to_String(riid));
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
	*ppvObject = nullptr;

	if (riid == IID_IUnknown || riid == IID_IClassFactory || riid == IID_ISpellCheckProviderFactory || riid == IID_Guid) {
		*ppvObject = this;
		hr = S_OK;
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE SpellerFactory::AddRef() {
	debugp p(__FUNCTION__);
	InterlockedIncrement(&refcount);
	return refcount;
}

ULONG STDMETHODCALLTYPE SpellerFactory::Release() {
	debugp p(__FUNCTION__);
	InterlockedDecrement(&refcount);
	return refcount;
}

IFACEMETHODIMP SpellerFactory::get_SupportedLanguages(_COM_Outptr_ IEnumString** value) {
	debugp p(__FUNCTION__);
	if (value == nullptr) {
		return E_POINTER;
	}
	*value = new EnumString(locales);
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
		spellers[locale] = std::make_unique<Speller>(locale);
	}
	*value = spellers[locale].get();

	return hr;
}
