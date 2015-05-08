/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#pragma once
#ifndef CLASSFACTORY_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b
#define CLASSFACTORY_HPP_e32d249d_a80d_4cb2_b414_7a61f946815b

#include "DLL.hpp"
#include <Unknwn.h>

class ClassFactory : public IClassFactory {
public:
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject);
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();

	HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown *pUnkOuter, REFIID riid, _COM_Outptr_ void **ppvObject);
	HRESULT STDMETHODCALLTYPE LockServer(BOOL fLock);

private:
	ULONG refcount = 1;
};

#endif
