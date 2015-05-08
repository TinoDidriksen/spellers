/*
	Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
	Licensed under the GNU GPL version 3 or later; see http://www.gnu.org/licenses/
*/
#include "Speller.hpp"
#include "DLL.hpp"
#include "EnumString.hpp"
#include <debugp.hpp>

const IID IID_ISpellCheckProvider = { 0x0C58F8DE, 0x8E94, 0x479E, { 0x97, 0x17, 0x70, 0xC4, 0x2C, 0x4A, 0xD2, 0xC3 } };

Speller::Speller(std::wstring locale_) :
locale(std::move(locale_))
{
	debugp p(__FUNCTION__);
	p(locale);

	SECURITY_ATTRIBUTES saAttr = { sizeof(saAttr), 0, true };

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
		p("SpellerInit CreatePipe 1");
		throw __LINE__;
	}
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
		p("SpellerInit SetHandleInformation 1");
		throw __LINE__;
	}
	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) {
		p("SpellerInit CreatePipe 2");
		throw __LINE__;
	}
	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
		p("SpellerInit SetHandleInformation 2");
		throw __LINE__;
	}

	PROCESS_INFORMATION piProcInfo = { 0 };
	STARTUPINFO siStartInfo = { sizeof(siStartInfo) };

	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	std::wstring wpath(conf["PATH"].begin(), conf["PATH"].end());
	while (!wpath.empty() && wpath.back() != '/' && wpath.back() != '\\') {
		wpath.pop_back();
	}

	wpath.append(L"backend");

	std::wstring cmdline(wpath);
	cmdline += '/';
	cmdline.append(conf["ENGINE"].begin(), conf["ENGINE"].end());
	cmdline.append(1, 0);

	BOOL bSuccess = CreateProcess(0,
		&cmdline[0],
		0,
		0,
		FALSE,
		CREATE_NO_WINDOW | DETACHED_PROCESS | BELOW_NORMAL_PRIORITY_CLASS,
		0,
		wpath.c_str(),
		&siStartInfo,
		&piProcInfo);

	if (!bSuccess) {
		std::string msg = conf["NAME"] + " could not start " + conf["ENGINE"] + "!\n\n";
		char *fmt = 0;
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPSTR)&fmt, 0, 0);
		msg += "GetLastError: ";
		msg += fmt;
		msg += '\n';
		LocalFree(fmt);
		MessageBoxA(0, msg.c_str(), "Speller launch error", MB_OK | MB_ICONERROR);
		throw __LINE__;
	}
	else {
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}

	DWORD bytes = 0, bytes_read = 0;
	cbuffer.resize(4);
	if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[0], 4, &bytes_read, 0) || bytes_read != 4) {
		p("checkValidWord ReadFile 1");
		throw __LINE__;
	}
	if (!PeekNamedPipe(g_hChildStd_OUT_Rd, 0, 0, 0, &bytes, 0)) {
		p("checkValidWord PeekNamedPipe");
		throw __LINE__;
	}
	if (bytes) {
		cbuffer.resize(4 + bytes);
		if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[4], bytes, &bytes_read, 0) || bytes != bytes_read) {
			p("checkValidWord ReadFile 2");
			throw __LINE__;
		}
	}

	cbuffer = trim(cbuffer);
	if (cbuffer != "@@ hfst-ospell-office is alive") {
		throw __LINE__;
	}
}

Speller::~Speller() {
	CloseHandle(g_hChildStd_IN_Rd);
	CloseHandle(g_hChildStd_IN_Wr);
	CloseHandle(g_hChildStd_OUT_Rd);
	CloseHandle(g_hChildStd_OUT_Wr);
}

HRESULT STDMETHODCALLTYPE Speller::QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject) {
	debugp p(__FUNCTION__);
	p(UUID_to_String(riid));
	if (ppvObject == nullptr) {
		return E_POINTER;
	}

	HRESULT hr = E_NOINTERFACE;
	*ppvObject = nullptr;

	if (riid == IID_IUnknown || riid == IID_ISpellCheckProvider) {
		*ppvObject = this;
		hr = S_OK;
		AddRef();
	}

	return hr;
}

ULONG STDMETHODCALLTYPE Speller::AddRef() {
	debugp p(__FUNCTION__);
	InterlockedIncrement(&refcount);
	p(refcount);
	return refcount;
}

ULONG STDMETHODCALLTYPE Speller::Release() {
	debugp p(__FUNCTION__);

	if (InterlockedDecrement(&refcount) == 0) {
		p(__LINE__);
		com_delete(this);
		return 0;
	}

	return refcount;
}

IFACEMETHODIMP Speller::get_LanguageTag(_Out_ PWSTR* value) {
	debugp p(__FUNCTION__);
	CoCopyWString(locale, value);
	return S_OK;
};

IFACEMETHODIMP Speller::get_Id(_Out_ PWSTR* value) {
	debugp p(__FUNCTION__);
	CoCopyWString(conf["NAME"], value);
	return S_OK;
}

IFACEMETHODIMP Speller::get_LocalizedName(_Out_ PWSTR* value) {
	debugp p(__FUNCTION__);
	CoCopyWString(conf["NAME"], value);
	return S_OK;
}

IFACEMETHODIMP Speller::get_OptionIds(_COM_Outptr_ IEnumString** value) {
	debugp p(__FUNCTION__);
	*value = com_new<EnumString>();
	return S_OK;
}

IFACEMETHODIMP Speller::Check(_In_ PCWSTR text, _COM_Outptr_ IEnumSpellingError** value) {
	debugp p(__FUNCTION__);
	if (text == nullptr) {
		return E_POINTER;
	}
	if (text[0] == 0) {
		return E_INVALIDARG;
	}

	*value = nullptr;

	std::wstring wtext(text);
	std::wstring word;
	size_t b = 0, e = 0;
	const wchar_t *spaces = L" \t\r\n!:;?{}\"";
	for (;;) {
		b = wtext.find_first_not_of(spaces, e);
		e = wtext.find_first_of(spaces, b);
		if (b == std::wstring::npos) {
			break;
		}

		word.assign(wtext.begin() + b, wtext.begin() + e);
		if (checkValidWord(word)) {
			continue;
		}

		// Store error offsets here

		if (e == std::wstring::npos) {
			break;
		}
	}
	return S_OK;
}

IFACEMETHODIMP Speller::Suggest(_In_ PCWSTR word, _COM_Outptr_ IEnumString** value) {
	debugp p(__FUNCTION__);
	if (word == nullptr) {
		return E_POINTER;
	}
	if (word[0] == 0) {
		return E_INVALIDARG;
	}

	*value = com_new<EnumString>();
	return S_OK;
}

IFACEMETHODIMP Speller::GetOptionValue(_In_ PCWSTR optionId, _Out_ BYTE* value) {
	debugp p(__FUNCTION__);
	return E_INVALIDARG;
}

IFACEMETHODIMP Speller::SetOptionValue(_In_ PCWSTR optionId, BYTE value) {
	debugp p(__FUNCTION__);
	return E_INVALIDARG;
}

IFACEMETHODIMP Speller::InitializeWordlist(WORDLIST_TYPE wordlistType, _In_ IEnumString* words) {
	debugp p(__FUNCTION__);
	HRESULT hr = S_OK;
	while (hr == S_OK) {
		LPOLESTR lpWord;
		hr = words->Next(1, &lpWord, nullptr);

		if (hr == S_OK) {
			p(wordlistType);
			p(std::wstring(lpWord));
			wordlists[wordlistType].insert(lpWord);
			CoTaskMemFree(lpWord);
		}
	}

	return hr;
}

IFACEMETHODIMP Speller::GetOptionDescription(_In_ PCWSTR optionId, _COM_Outptr_ IOptionDescription** value) {
	debugp p(__FUNCTION__);
	*value = nullptr;
	return E_INVALIDARG;
}

bool Speller::checkValidWord(const std::wstring& word, size_t suggs) {
	debugp p("checkValidWord");
	p(word);

	if (valid_words.find(word) != valid_words.end()) {
		return true;
	}
	if (invalid_words.find(word) != invalid_words.end()) {
		return false;
	}

	cbuffer.resize(std::numeric_limits<size_t>::digits10 + 2);
	cbuffer.resize(sprintf(&cbuffer[0], "%llu ", static_cast<unsigned long long>(suggs)));
	size_t off = cbuffer.size();
	cbuffer.resize(cbuffer.size() + word.size() * 4);
	cbuffer.resize(WideCharToMultiByte(CP_UTF8, 0, word.c_str(), word.size(), &cbuffer[off], cbuffer.size() - off, 0, 0) + off);
	cbuffer += '\n';

	DWORD bytes = 0, bytes_read = 0;
	if (!WriteFile(g_hChildStd_IN_Wr, cbuffer.c_str(), cbuffer.size(), &bytes, 0) || bytes != cbuffer.size()) {
		p("checkValidWord WriteFile");
		return false;
	}
	cbuffer.resize(1);
	if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[0], 1, &bytes_read, 0) || bytes_read != 1) {
		p("checkValidWord ReadFile 1");
		return false;
	}
	if (!PeekNamedPipe(g_hChildStd_OUT_Rd, 0, 0, 0, &bytes, 0)) {
		p("checkValidWord PeekNamedPipe");
		return false;
	}
	if (bytes) {
		cbuffer.resize(1 + bytes);
		if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[1], bytes, &bytes_read, 0) || bytes != bytes_read) {
			p("checkValidWord ReadFile 2");
			return false;
		}
	}

	cbuffer = trim(cbuffer);

	if (cbuffer[0] == '*') {
		valid_words.insert(word);
		return true;
	}

	if (suggs == 0) {
		return false;
	}

	std::vector<std::wstring>& alts = invalid_words[word];
	alts.clear();

	// This includes # for no alternatives and ! for error
	if (cbuffer[0] != '&') {
		return false;
	}

	wbuffer.resize(cbuffer.size() * 2);
	wbuffer.resize(MultiByteToWideChar(CP_UTF8, 0, cbuffer.c_str(), cbuffer.size(), &wbuffer[0], wbuffer.size()));

	size_t e = wbuffer.find(L"\t");
	if (e == std::wstring::npos) {
		return false;
	}

	size_t b = e + 1;
	while ((e = wbuffer.find(L"\t", b)) != std::wstring::npos) {
		alts.emplace_back(wbuffer.begin() + b, wbuffer.begin() + e);
		b = e + 1;
	}
	alts.emplace_back(wbuffer.begin() + b, wbuffer.end());

	return false;
}
