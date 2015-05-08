/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#include "Speller.hpp"
#include "DLL.hpp"
#include "EnumString.hpp"
#include <debugp.hpp>

const IID IID_ISpellCheckProvider = { 0x0C58F8DE, 0x8E94, 0x479E, { 0x97, 0x17, 0x70, 0xC4, 0x2C, 0x4A, 0xD2, 0xC3 } };

Speller::Speller(std::wstring locale_) :
locale(std::move(locale_))
{
	debugp p(__FUNCTION__);
	p(locale);
}

HRESULT STDMETHODCALLTYPE Speller::QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject) {
	debugp p(__FUNCTION__);
	p(UUID_to_String(riid));
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	HRESULT hr = E_NOINTERFACE;
	*ppvObject = nullptr;

	if (riid == IID_IUnknown || riid == IID_ISpellCheckProvider) {
		*ppvObject = this;
		hr = S_OK;
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE Speller::AddRef() {
	debugp p(__FUNCTION__);
	InterlockedIncrement(&refcount);
	p(refcount);
	return refcount;
}

ULONG STDMETHODCALLTYPE Speller::Release() {
	debugp p(__FUNCTION__);

	if (InterlockedDecrement(&refcount) == 0) {
		p(__LINE__);
		com_delete(this);
		return 0;
	}

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
	*value = com_new<EnumString>();
	return S_OK;
}

IFACEMETHODIMP Speller::Check(_In_ PCWSTR text, _COM_Outptr_ IEnumSpellingError** value) {
	debugp p(__FUNCTION__);
	*value = nullptr;
	return S_OK;
}

IFACEMETHODIMP Speller::Suggest(_In_ PCWSTR word, _COM_Outptr_ IEnumString** value) {
	debugp p(__FUNCTION__);
	*value = com_new<EnumString>();
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
	while (hr == S_OK) {
		LPOLESTR lpWord;
		hr = words->Next(1, &lpWord, nullptr);

		if (hr == S_OK) {
			p(wordlistType);
			p(std::wstring(lpWord));
			wordlists[wordlistType].insert(lpWord);
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
