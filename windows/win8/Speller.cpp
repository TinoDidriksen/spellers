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

const IID IID_ISpellCheckProvider = { 0x0C58F8DE, 0x8E94, 0x479E, { 0x97, 0x17, 0x70, 0xC4, 0x2C, 0x4A, 0xD2, 0xC3 } };

Speller::Speller(std::wstring locale_) :
locale(std::move(locale_))
{
	debugp p(__FUNCTION__);
	p(locale);
}

Speller::~Speller() {
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
	if (text == nullptr) {
		p("nullptr");
		return E_POINTER;
	}
	if (text[0] == 0) {
		p("empty");
		return E_INVALIDARG;
	}

	*value = nullptr;

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

		// Store error offsets here

		if (e >= wtext.size()) {
			break;
		}
	}
	return S_OK;
}

IFACEMETHODIMP Speller::Suggest(_In_ PCWSTR word, _COM_Outptr_ IEnumString** value) {
	debugp p(__FUNCTION__);
	if (word == nullptr) {
		return E_POINTER;
	}
	if (word[0] == 0) {
		return E_INVALIDARG;
	}

	*value = com_new<EnumString>();
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

IFACEMETHODIMP Speller::GetOptionDescription(_In_ PCWSTR optionId, _COM_Outptr_ IOptionDescription** value) {
	debugp p(__FUNCTION__);
	(void)optionId;
	*value = nullptr;
	return E_INVALIDARG;
}

bool Speller::checkValidWord(const std::wstring& word, size_t suggs) {
	debugp p("checkValidWord");
	p(word);

	if (valid_words.find(word) != valid_words.end()) {
		return true;
	}
	if (invalid_words.find(word) != invalid_words.end()) {
		return false;
	}

	cbuffer.resize(std::numeric_limits<size_t>::digits10 + 2);
	cbuffer.resize(sprintf(&cbuffer[0], "%llu ", static_cast<unsigned long long>(suggs)));
	size_t off = cbuffer.size();
	cbuffer.resize(cbuffer.size() + word.size() * 4);
	cbuffer.resize(WideCharToMultiByte(CP_UTF8, 0, word.c_str(), static_cast<int>(word.size()), &cbuffer[off], static_cast<int>(cbuffer.size() - off), 0, 0) + off);
	cbuffer += '\n';

	HANDLE pipe = CreateFileA(conf["PIPE"].c_str(), GENERIC_READ | FILE_WRITE_DATA, 0, nullptr, OPEN_EXISTING, 0, nullptr);

	if (pipe == INVALID_HANDLE_VALUE) {
		p("CreateFileA failed", GetLastError());
		return false;
	}

	p("Writing pipe", cbuffer);
	DWORD bytes = 0;
	DWORD bytes_read = 0;
	if (!WriteFile(pipe, cbuffer.c_str(), static_cast<DWORD>(cbuffer.size()), &bytes, 0) || bytes != cbuffer.size()) {
		p("WriteFile(pipe)", GetLastError());
		return false;
	}

	p("Reading pipe");
	bytes = 0;
	bytes_read = 0;
	cbuffer.resize(1);
	if (!ReadFile(pipe, &cbuffer[0], 1, &bytes_read, 0) || bytes_read != 1) {
		p("ReadFile(pipe) 1", GetLastError());
		return false;
	}
	if (!PeekNamedPipe(pipe, 0, 0, 0, &bytes, 0)) {
		p("PeekNamedPipe(pipe)", GetLastError());
		return false;
	}
	if (bytes) {
		cbuffer.resize(1 + bytes);
		if (!ReadFile(pipe, &cbuffer[1], bytes, &bytes_read, 0) || bytes != bytes_read) {
			p("ReadFile(pipe) 2", GetLastError());
			return false;
		}
	}

	cbuffer = trim(cbuffer);

	if (cbuffer[0] == '*') {
		valid_words.insert(word);
		return true;
	}

	if (suggs == 0) {
		return false;
	}

	std::vector<std::wstring>& alts = invalid_words[word];
	alts.clear();

	// This includes # for no alternatives and ! for error
	if (cbuffer[0] != '&') {
		return false;
	}

	wbuffer.resize(cbuffer.size() * 2);
	wbuffer.resize(MultiByteToWideChar(CP_UTF8, 0, cbuffer.c_str(), static_cast<int>(cbuffer.size()), &wbuffer[0], static_cast<int>(wbuffer.size())));

	size_t e = wbuffer.find(L"\t");
	if (e == std::wstring::npos) {
		return false;
	}

	size_t b = e + 1;
	while ((e = wbuffer.find(L"\t", b)) != std::wstring::npos) {
		alts.emplace_back(wbuffer.begin() + b, wbuffer.begin() + e);
		b = e + 1;
	}
	alts.emplace_back(wbuffer.begin() + b, wbuffer.end());

	return false;
}
