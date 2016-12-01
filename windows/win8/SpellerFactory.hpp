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
#ifndef SPELLERFACTORY_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define SPELLERFACTORY_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include "Speller.hpp"
#include "DLL.hpp"
#include <map>
#include <string>
#include <memory>
#include <spellcheckprovider.h>

extern const IID IID_ISpellCheckProviderFactory;

class SpellerFactory : public ISpellCheckProviderFactory {
public:
	~SpellerFactory();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	IFACEMETHOD(IsSupported)(_In_ PCWSTR languageTag, _Out_ BOOL* value);
	IFACEMETHOD(CreateSpellCheckProvider)(_In_ PCWSTR languageTag, ISpellCheckProvider** value);

	IFACEMETHOD(get_SupportedLanguages)(IEnumString** value);
private:
	ULONG refcount = 1;
	std::map<std::wstring, Speller*> spellers;
};

#endif
