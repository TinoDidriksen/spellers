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

/*
* Auto-generated from interfaces used by the Windows code
*/

#pragma once
#ifndef __spellcheckprovider_h__
#define __spellcheckprovider_h__

#include "rpc.h"
#include "rpcndr.h"

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif

#ifndef __ISpellingError_FWD_DEFINED__
	#define __ISpellingError_FWD_DEFINED__
	typedef interface ISpellingError ISpellingError;
#endif

#ifndef __IEnumSpellingError_FWD_DEFINED__
	#define __IEnumSpellingError_FWD_DEFINED__
	typedef interface IEnumSpellingError IEnumSpellingError;
#endif

#ifndef __IOptionDescription_FWD_DEFINED__
	#define __IOptionDescription_FWD_DEFINED__
	typedef interface IOptionDescription IOptionDescription;
#endif

#ifndef __ISpellCheckProvider_FWD_DEFINED__
	#define __ISpellCheckProvider_FWD_DEFINED__
	typedef interface ISpellCheckProvider ISpellCheckProvider;
#endif

#ifndef __IComprehensiveSpellCheckProvider_FWD_DEFINED__
	#define __IComprehensiveSpellCheckProvider_FWD_DEFINED__
	typedef interface IComprehensiveSpellCheckProvider IComprehensiveSpellCheckProvider;
#endif

#ifndef __ISpellCheckProviderFactory_FWD_DEFINED__
	#define __ISpellCheckProviderFactory_FWD_DEFINED__
	typedef interface ISpellCheckProviderFactory ISpellCheckProviderFactory;
#endif

extern "C" {

#ifndef __ISpellingError_INTERFACE_DEFINED__
#define __ISpellingError_INTERFACE_DEFINED__

typedef
enum WORDLIST_TYPE
    {
        WORDLIST_TYPE_IGNORE	= 0,
        WORDLIST_TYPE_ADD	= 1,
        WORDLIST_TYPE_EXCLUDE	= 2,
        WORDLIST_TYPE_AUTOCORRECT	= 3
    } 	WORDLIST_TYPE;

typedef
enum CORRECTIVE_ACTION
    {
        CORRECTIVE_ACTION_NONE	= 0,
        CORRECTIVE_ACTION_GET_SUGGESTIONS	= 1,
        CORRECTIVE_ACTION_REPLACE	= 2,
        CORRECTIVE_ACTION_DELETE	= 3
    } 	CORRECTIVE_ACTION;

EXTERN_C const IID IID_ISpellingError;

MIDL_INTERFACE("B7C82D61-FBE8-4B47-9B27-6C0D2E0DE0A3")
ISpellingError : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE get_StartIndex(
		__RPC__out ULONG *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_Length(
		__RPC__out ULONG *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_CorrectiveAction(
		__RPC__out CORRECTIVE_ACTION *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_Replacement(
		__RPC__deref_out_opt LPWSTR *value) = 0;
};

#endif


#ifndef __IEnumSpellingError_INTERFACE_DEFINED__
#define __IEnumSpellingError_INTERFACE_DEFINED__

EXTERN_C const IID IID_IEnumSpellingError;

MIDL_INTERFACE("803E3BD4-2828-4410-8290-418D1D73C762")
IEnumSpellingError : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Next(
		__RPC__deref_out_opt ISpellingError **value) = 0;
};

#endif


#ifndef __IOptionDescription_INTERFACE_DEFINED__
#define __IOptionDescription_INTERFACE_DEFINED__

EXTERN_C const IID IID_IOptionDescription;

MIDL_INTERFACE("432E5F85-35CF-4606-A801-6F70277E1D7A")
IOptionDescription : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE get_Id(
		__RPC__deref_out_opt LPWSTR *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_Heading(
		__RPC__deref_out_opt LPWSTR *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_Description(
		__RPC__deref_out_opt LPWSTR *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_Labels(
		__RPC__deref_out_opt IEnumString **value) = 0;
};

#endif


#ifndef __ISpellCheckProvider_INTERFACE_DEFINED__
#define __ISpellCheckProvider_INTERFACE_DEFINED__

EXTERN_C const IID IID_ISpellCheckProvider;

MIDL_INTERFACE("73E976E0-8ED4-4EB1-80D7-1BE0A16B0C38")
ISpellCheckProvider : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE get_LanguageTag(
		__RPC__deref_out_opt LPWSTR *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE Check(
		__RPC__in LPCWSTR text,
		__RPC__deref_out_opt IEnumSpellingError **value) = 0;

	virtual HRESULT STDMETHODCALLTYPE Suggest(
		__RPC__in LPCWSTR word,
		__RPC__deref_out_opt IEnumString **value) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetOptionValue(
		__RPC__in LPCWSTR optionId,
		__RPC__out BYTE *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetOptionValue(
		__RPC__in LPCWSTR optionId,
		BYTE value) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_OptionIds(
		__RPC__deref_out_opt IEnumString **value) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_Id(
		__RPC__deref_out_opt LPWSTR *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE get_LocalizedName(
		__RPC__deref_out_opt LPWSTR *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetOptionDescription(
		__RPC__in LPCWSTR optionId,
		__RPC__deref_out_opt IOptionDescription **value) = 0;

	virtual HRESULT STDMETHODCALLTYPE InitializeWordlist(
		WORDLIST_TYPE wordlistType,
		__RPC__in_opt IEnumString *words) = 0;
};

#endif


#ifndef __IComprehensiveSpellCheckProvider_INTERFACE_DEFINED__
#define __IComprehensiveSpellCheckProvider_INTERFACE_DEFINED__

EXTERN_C const IID IID_IComprehensiveSpellCheckProvider;

MIDL_INTERFACE("0C58F8DE-8E94-479E-9717-70C42C4AD2C3")
IComprehensiveSpellCheckProvider : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE ComprehensiveCheck(
		__RPC__in LPCWSTR text,
		__RPC__deref_out_opt IEnumSpellingError **value) = 0;
};

#endif


#ifndef __ISpellCheckProviderFactory_INTERFACE_DEFINED__
#define __ISpellCheckProviderFactory_INTERFACE_DEFINED__

EXTERN_C const IID IID_ISpellCheckProviderFactory;

MIDL_INTERFACE("9F671E11-77D6-4C92-AEFB-615215E3A4BE")
ISpellCheckProviderFactory : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE get_SupportedLanguages(
		__RPC__deref_out_opt IEnumString **value) = 0;

	virtual HRESULT STDMETHODCALLTYPE IsSupported(
		__RPC__in LPCWSTR languageTag,
		__RPC__out BOOL *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE CreateSpellCheckProvider(
		__RPC__in LPCWSTR languageTag,
		__RPC__deref_out_opt ISpellCheckProvider **value) = 0;
};

#endif

}

#endif
