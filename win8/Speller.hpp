/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#pragma once
#ifndef SPELLER_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define SPELLER_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include <string>
#include <spellcheckprovider.h>
#include <windows.h>

class DECLSPEC_UUID("73E976E0-8ED4-4EB1-80D7-1BE0A16B0C38") Speller : public ISpellCheckProvider{
public:
	Speller(std::wstring locale);

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
	ULONG refcount = 1;
	std::wstring locale;
};

#endif
