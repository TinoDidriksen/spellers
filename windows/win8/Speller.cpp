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

#include "Speller.hpp"
#include "DLL.hpp"
#include "EnumString.hpp"
#include <shared.hpp>
#include <debugp.hpp>
#include <shellapi.h>
#include <cwchar>

const IID IID_ISpellCheckProviderFactory = { 0x9F671E11, 0x77D6, 0x4C92, { 0xAE, 0xFB, 0x61, 0x52, 0x15, 0xE3, 0xA4, 0xBE } };
const IID IID_ISpellCheckProvider = { 0x0C58F8DE, 0x8E94, 0x479E, { 0x97, 0x17, 0x70, 0xC4, 0x2C, 0x4A, 0xD2, 0xC3 } };
const IID IID_ISpellingError = { 0xB7C82D61, 0xFBE8, 0x4B47, { 0x9B, 0x27, 0x6C, 0x0D, 0x2E, 0x0D, 0xE0, 0xA3 } };
const IID IID_IEnumSpellingError = { 0x803E3BD4, 0x2828, 0x4410, { 0x82, 0x90, 0x41, 0x8D, 0x1D, 0x73, 0xC7, 0x62 } };

Speller::Speller(std::wstring locale_)
	: locale(std::move(locale_))
{
	debugp p(__FUNCTION__);
	p(locale);

	std::string engine = conf["PATH"] + conf["ENGINE"].substr(conf["ENGINE"].find(' ') + 1);
	p(engine);
	try {
		speller.read_zhfst(engine);
		speller.set_time_cutoff(6.0);
	}
	catch (hfst_ol::ZHfstMetaDataParsingError ex) {
		p("ZHfstMetaDataParsingError:", ex.what());
		throw;
	}
	catch (hfst_ol::ZHfstZipReadingError ex) {
		p("ZHfstZipReadingError:", ex.what());
		throw;
	}
	catch (hfst_ol::ZHfstXmlParsingError ex) {
		p("ZHfstXmlParsingError:", ex.what());
		throw;
	}
}

Speller::~Speller() {
}

HRESULT STDMETHODCALLTYPE Speller::QueryInterface(REFIID riid, void **ppvObject) {
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
		p("Cleanup", __LINE__);
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

IFACEMETHODIMP Speller::get_OptionIds(IEnumString** value) {
	debugp p(__FUNCTION__);
	*value = com_new<EnumString>();
	return S_OK;
}

IFACEMETHODIMP Speller::Check(_In_ PCWSTR text, IEnumSpellingError** value) {
	debugp p(__FUNCTION__);
	if (text == nullptr) {
		p("nullptr");
		return E_POINTER;
	}
	if (text[0] == 0) {
		p("empty");
		return E_INVALIDARG;
	}

	std::vector<SpellingError> errors;

	std::wstring wtext(text);
	p(wtext);
	std::wstring word;
	size_t b = 0, e = 0;
	const wchar_t *spaces = L" \t\r\n!:;?{}\"";
	for (;;) {
		b = wtext.find_first_not_of(spaces, e);
		e = wtext.find_first_of(spaces, b);
		if (b == std::wstring::npos) {
			break;
		}
		if (e == std::wstring::npos) {
			e = wtext.size();
		}

		word.assign(wtext.begin() + b, wtext.begin() + e);
		if (checkValidWord(word)) {
			continue;
		}

		errors.emplace_back(SpellingError(b, e - b));

		if (e >= wtext.size()) {
			break;
		}
	}

	*value = com_new<EnumSpellingError>(std::move(errors));
	return S_OK;
}

IFACEMETHODIMP Speller::Suggest(_In_ PCWSTR word, IEnumString** value) {
	debugp p(__FUNCTION__);
	if (word == nullptr) {
		return E_POINTER;
	}
	if (word[0] == 0) {
		return E_INVALIDARG;
	}

	if (checkValidWord(word, 10)) {
		*value = com_new<EnumString>(word);
		return S_FALSE;
	}

	*value = com_new<EnumString>(invalid_words[word]);
	return S_OK;
}

IFACEMETHODIMP Speller::GetOptionValue(_In_ PCWSTR optionId, _Out_ BYTE* value) {
	debugp p(__FUNCTION__);
	(void)optionId;
	(void)value;
	return E_INVALIDARG;
}

IFACEMETHODIMP Speller::SetOptionValue(_In_ PCWSTR optionId, BYTE value) {
	debugp p(__FUNCTION__);
	(void)optionId;
	(void)value;
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

IFACEMETHODIMP Speller::GetOptionDescription(_In_ PCWSTR optionId, IOptionDescription** value) {
	debugp p(__FUNCTION__);
	(void)optionId;
	*value = nullptr;
	return E_INVALIDARG;
}

bool Speller::checkValidWord(const std::wstring& word, size_t suggs) {
	debugp p("checkValidWord");
	p(word, suggs);

	if (valid_words.find(word) != valid_words.end()) {
		return true;
	}
	if (invalid_words.find(word) != invalid_words.end()) {
		return false;
	}

	w2n(word, cbuffer);

	if (speller.spell(cbuffer)) {
		valid_words.insert(word);
		return true;
	}

	if (suggs == 0) {
		return false;
	}

	std::vector<std::wstring>& alts = invalid_words[word];
	alts.clear();

	hfst_ol::CorrectionQueue corrections = speller.suggest(cbuffer);

	if (corrections.size() == 0) {
		return false;
	}

	for (size_t i = 0, e = corrections.size() ; i < e && i < suggs ; ++i) {
		const std::string& narrow = corrections.top().first;

		n2w(narrow, wbuffer);
		alts.emplace_back(std::move(wbuffer));

		corrections.pop();
	}

	return false;
}
