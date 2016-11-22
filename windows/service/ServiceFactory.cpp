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

#include "ServiceFactory.hpp"
#include "service.hpp"
#include <COM.hpp>
#include <debugp.hpp>

HRESULT STDMETHODCALLTYPE ServiceFactory::QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject) {
	debugp p(__FUNCTION__);
	p(UUID_to_String(riid));
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
	*ppvObject = nullptr;

	if (riid == IID_IUnknown || riid == IID_Guid) {
		*ppvObject = this;
		hr = S_OK;
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE ServiceFactory::AddRef() {
	debugp p(__FUNCTION__);
	InterlockedIncrement(&refcount);
	return refcount;
}

ULONG STDMETHODCALLTYPE ServiceFactory::Release() {
	debugp p(__FUNCTION__);
	InterlockedDecrement(&refcount);
	return refcount;
}

HRESULT STDMETHODCALLTYPE ServiceFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, _COM_Outptr_ void **ppvObject) {
	debugp p(__FUNCTION__);
	p(UUID_to_String(riid));
	if (pUnkOuter) {
		return CLASS_E_NOAGGREGATION;
	}
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	HRESULT hr = E_NOINTERFACE;

	if (riid == IID_IUnknown || riid == IID_Guid) {
		auto ptr = com_new<ServiceFactory>();
		hr = ptr->QueryInterface(riid, ppvObject);
		ptr->Release();
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE ServiceFactory::LockServer(BOOL fLock) {
	(void)fLock;
	return S_OK;
}
