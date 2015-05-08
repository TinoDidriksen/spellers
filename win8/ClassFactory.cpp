/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#include "ClassFactory.hpp"
#include "SpellerFactory.hpp"
#include <debugp.hpp>

HRESULT STDMETHODCALLTYPE ClassFactory::QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject) {
	debugp p(__FUNCTION__);
	p(UUID_to_String(riid));
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
	*ppvObject = nullptr;

	if (riid == IID_IUnknown || riid == IID_IClassFactory || riid == IID_Guid) {
		*ppvObject = this;
		hr = S_OK;
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE ClassFactory::AddRef() {
	debugp p(__FUNCTION__);
	InterlockedIncrement(&refcount);
	return refcount;
}

ULONG STDMETHODCALLTYPE ClassFactory::Release() {
	debugp p(__FUNCTION__);
	InterlockedDecrement(&refcount);
	return refcount;
}

HRESULT STDMETHODCALLTYPE ClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, _COM_Outptr_ void **ppvObject) {
	debugp p(__FUNCTION__);
	p(UUID_to_String(riid));
	if (pUnkOuter) {
		return CLASS_E_NOAGGREGATION;
	}
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	HRESULT hr = E_NOINTERFACE;

	if (riid == IID_IUnknown || riid == IID_ISpellCheckProviderFactory || riid == IID_Guid) {
		auto ptr = com_new<SpellerFactory>();
		hr = ptr->QueryInterface(riid, ppvObject);
		ptr->Release();
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE ClassFactory::LockServer(BOOL fLock) {
	return S_OK;
}
