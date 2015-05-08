/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
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

	HANDLE g_hChildStd_IN_Rd = 0;
	HANDLE g_hChildStd_IN_Wr = 0;
	HANDLE g_hChildStd_OUT_Rd = 0;
	HANDLE g_hChildStd_OUT_Wr = 0;
	std::unordered_set<std::wstring> valid_words;
	std::unordered_map<std::wstring, std::vector<std::wstring>> invalid_words;
	std::string cbuffer;
	std::wstring wbuffer;
};

#endif
