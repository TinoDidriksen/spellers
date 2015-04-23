/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#pragma once
#ifndef SPELLERFACTORY_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define SPELLERFACTORY_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include "Speller.hpp"
#include <map>
#include <string>
#include <memory>
#include <spellcheckprovider.h>
#include <windows.h>

class SpellerFactory : public ISpellCheckProviderFactory {
public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	IFACEMETHOD(IsSupported)(_In_ PCWSTR languageTag, _Out_ BOOL* value);
	IFACEMETHOD(CreateSpellCheckProvider)(_In_ PCWSTR languageTag, _COM_Outptr_ ISpellCheckProvider** value);

	IFACEMETHOD(get_SupportedLanguages)(_COM_Outptr_ IEnumString** value);

private:
	ULONG refcount = 1;
	std::map<std::wstring, std::unique_ptr<Speller>> spellers;
};

#endif
