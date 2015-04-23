/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#include "Speller.hpp"
#include "DLL.hpp"
#include "EnumString.hpp"
#include <debugp.hpp>

Speller::Speller(std::wstring locale) :
locale(std::move(locale))
{
}

HRESULT STDMETHODCALLTYPE Speller::QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject) {
	debugp p(__FUNCTION__);
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	HRESULT hr = E_NOINTERFACE;
	*ppvObject = nullptr;

	if (riid == IID_IUnknown /*|| riid == IID_ISpellCheckProvider*/ || riid == IID_Guids[GUID_Speller]) {
		*ppvObject = this;
		hr = S_OK;
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE Speller::AddRef() {
	debugp p(__FUNCTION__);
	InterlockedIncrement(&refcount);
	return refcount;
}

ULONG STDMETHODCALLTYPE Speller::Release() {
	debugp p(__FUNCTION__);
	InterlockedDecrement(&refcount);
	return refcount;
}

IFACEMETHODIMP Speller::get_LanguageTag(_Out_ PWSTR* value) {
	debugp p(__FUNCTION__);
	CoCopyWString(locale, value);
	return S_OK;
};

IFACEMETHODIMP Speller::get_Id(_Out_ PWSTR* value) {
	debugp p(__FUNCTION__);
	CoCopyWString(conf["NAME"], value);
	return S_OK;
}

IFACEMETHODIMP Speller::get_LocalizedName(_Out_ PWSTR* value) {
	debugp p(__FUNCTION__);
	CoCopyWString(conf["NAME"], value);
	return S_OK;
}

IFACEMETHODIMP Speller::get_OptionIds(_COM_Outptr_ IEnumString** value) {
	debugp p(__FUNCTION__);
	*value = nullptr;
	return S_OK;
}

IFACEMETHODIMP Speller::Check(_In_ PCWSTR text, _COM_Outptr_ IEnumSpellingError** value) {
	debugp p(__FUNCTION__);
	*value = nullptr;
	return S_OK;
}

IFACEMETHODIMP Speller::Suggest(_In_ PCWSTR word, _COM_Outptr_ IEnumString** value) {
	debugp p(__FUNCTION__);
	*value = nullptr;
	return S_OK;
}

IFACEMETHODIMP Speller::GetOptionValue(_In_ PCWSTR optionId, _Out_ BYTE* value) {
	debugp p(__FUNCTION__);
	return E_INVALIDARG;
}

IFACEMETHODIMP Speller::SetOptionValue(_In_ PCWSTR optionId, BYTE value) {
	debugp p(__FUNCTION__);
	return E_INVALIDARG;
}

IFACEMETHODIMP Speller::InitializeWordlist(WORDLIST_TYPE wordlistType, _In_ IEnumString* words) {
	debugp p(__FUNCTION__);
	HRESULT hr = S_OK;
	while (S_OK == hr) {
		LPOLESTR lpWord;
		hr = words->Next(1, &lpWord, nullptr);

		if (S_OK == hr) {
			CoTaskMemFree(lpWord);
		}
	}

	return hr;
}

IFACEMETHODIMP Speller::GetOptionDescription(_In_ PCWSTR optionId, _COM_Outptr_ IOptionDescription** value) {
	debugp p(__FUNCTION__);
	*value = nullptr;
	return E_INVALIDARG;
}
