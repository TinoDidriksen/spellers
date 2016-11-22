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
#ifndef SPELLER_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define SPELLER_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include <vector>
#include <string>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <spellcheckprovider.h>
#include <windows.h>

class Speller : public ISpellCheckProvider{
public:
	Speller(std::wstring locale);
	~Speller();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	IFACEMETHOD(Check)(_In_ PCWSTR text, _COM_Outptr_ IEnumSpellingError** value);
	IFACEMETHOD(Suggest)(_In_ PCWSTR word, _COM_Outptr_ IEnumString** value);
	IFACEMETHOD(InitializeWordlist)(WORDLIST_TYPE wordlistType, _In_ IEnumString* words);
	IFACEMETHOD(GetOptionValue)(_In_ PCWSTR optionId, _Out_ BYTE* value);
	IFACEMETHOD(SetOptionValue)(_In_ PCWSTR optionId, BYTE value);
	IFACEMETHOD(GetOptionDescription)(_In_ PCWSTR optionId, _COM_Outptr_ IOptionDescription** value);

	IFACEMETHOD(get_LanguageTag)(_Out_ PWSTR* value);
	IFACEMETHOD(get_Id)(_Out_ PWSTR* value);
	IFACEMETHOD(get_LocalizedName)(_Out_ PWSTR* value);
	IFACEMETHOD(get_OptionIds)(_COM_Outptr_ IEnumString** value);

private:
	bool checkValidWord(const std::wstring& word, size_t suggs = 0);

	ULONG refcount = 1;
	std::wstring locale;
	std::map<WORDLIST_TYPE, std::unordered_set<std::wstring>> wordlists;

	std::unordered_set<std::wstring> valid_words;
	std::unordered_map<std::wstring, std::vector<std::wstring>> invalid_words;
	std::string cbuffer;
	std::wstring wbuffer;
};

#endif
