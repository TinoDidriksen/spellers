/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
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
#include <atlbase.h>
#include <atlcom.h>

class DECLSPEC_UUID("A3AC5A53-231B-44A2-A28B-E59CCA979FB0") SpellerFactory : public ISpellCheckProviderFactory
	, public ATL::CComCoClass<SpellerFactory, &IID_Guid> // ATL implementation for CreateInstance, etc...
	, public ATL::CComObjectRootEx<ATL::CComMultiThreadModelNoCS> // ATL implementation for IUnknown
{
public:
	~SpellerFactory();

	IFACEMETHOD(IsSupported)(_In_ PCWSTR languageTag, _Out_ BOOL* value);
	IFACEMETHOD(CreateSpellCheckProvider)(_In_ PCWSTR languageTag, _COM_Outptr_ ISpellCheckProvider** value);

	IFACEMETHOD(get_SupportedLanguages)(_COM_Outptr_ IEnumString** value);

	DECLARE_REGISTRY_RESOURCEID(101)
	BEGIN_COM_MAP(SpellerFactory)
		COM_INTERFACE_ENTRY(ISpellCheckProviderFactory)
	END_COM_MAP()

	DECLARE_NOT_AGGREGATABLE(SpellerFactory)
private:
	ULONG refcount = 1;
	std::map<std::wstring, Speller*> spellers;
};

OBJECT_ENTRY_AUTO(__uuidof(SpellerFactory), SpellerFactory)

#endif
