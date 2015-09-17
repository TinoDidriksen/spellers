/*
* Copyright (C) 2015, Tino Didriksen <mail@tinodidriksen.com>
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

#include <vector>
#include <set>
#include <string>
#include <cstdio>
#include <map>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <windows.h>
#include <sddl.h>
#include <shared.hpp>
#include <debugp.hpp>

#ifndef NDEBUG
std::ofstream debug("C:/Temp/Tino/debug-speller-service.txt");
thread_local size_t debugd = 0;
#endif

HANDLE g_hChildStd_IN_Rd = 0;
HANDLE g_hChildStd_IN_Wr = 0;
HANDLE g_hChildStd_OUT_Rd = 0;
HANDLE g_hChildStd_OUT_Wr = 0;

std::map<std::string, std::string> conf;
std::string cbuffer;

int hfst_init() {
	debugp p(__FUNCTION__);
	SECURITY_ATTRIBUTES saAttr = {sizeof(saAttr), 0, true};

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0)) {
		p("CreatePipe 1", GetLastError());
		return -__LINE__;
	}
	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0)) {
		p("SetHandleInformation 1", GetLastError());
		return -__LINE__;
	}
	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0)) {
		p("CreatePipe 2", GetLastError());
		return -__LINE__;
	}
	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0)) {
		p("SetHandleInformation 2", GetLastError());
		return -__LINE__;
	}

	PROCESS_INFORMATION piProcInfo = {0};
	STARTUPINFOA siStartInfo = {sizeof(siStartInfo)};

	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	std::string path = conf["PATH"];
	path.append("backend");

	std::string cmdline(path);
	cmdline += '/';
	cmdline.append(conf["ENGINE"].begin(), conf["ENGINE"].end());
	cmdline.append(1, 0);

	BOOL bSuccess = CreateProcessA(0,
		&cmdline[0],
		0,
		0,
		TRUE,
		CREATE_NO_WINDOW | BELOW_NORMAL_PRIORITY_CLASS,
		0,
		path.c_str(),
		&siStartInfo,
		&piProcInfo);

	if (!bSuccess) {
		p("CreateProcessA failed", GetLastError());
		return -__LINE__;
	}
	else {
		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}

	DWORD bytes = 0, bytes_read = 0;
	cbuffer.resize(4);
	if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[0], 4, &bytes_read, 0) || bytes_read != 4) {
		p("shim_init ReadFile 1", GetLastError());
		return -__LINE__;
	}
	if (!PeekNamedPipe(g_hChildStd_OUT_Rd, 0, 0, 0, &bytes, 0)) {
		p("shim_init PeekNamedPipe", GetLastError());
		return -__LINE__;
	}
	if (bytes) {
		cbuffer.resize(4 + bytes);
		if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[4], bytes, &bytes_read, 0) || bytes != bytes_read) {
			p("shim_init ReadFile 2", GetLastError());
		return -__LINE__;
		}
	}

	cbuffer = trim(cbuffer);
	if (cbuffer != "@@ hfst-ospell-office is alive") {
		return -__LINE__;
	}

	return 0;
}

void hfst_terminate() {
	debugp p(__FUNCTION__);
	CloseHandle(g_hChildStd_IN_Rd);
	CloseHandle(g_hChildStd_IN_Wr);
	CloseHandle(g_hChildStd_OUT_Rd);
	CloseHandle(g_hChildStd_OUT_Wr);
}

SERVICE_STATUS g_svcStatus = {};
SERVICE_STATUS_HANDLE g_svcHandle = nullptr;
HANDLE g_svcEvent = INVALID_HANDLE_VALUE;

void WINAPI service_control(DWORD fdwControl) {
	debugp p(__FUNCTION__);

	switch (fdwControl) {
	case SERVICE_CONTROL_STOP:
		p("SERVICE_CONTROL_STOP");
		if (g_svcStatus.dwCurrentState != SERVICE_RUNNING) {
			break;
		}

		g_svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_svcStatus.dwControlsAccepted = 0;
		g_svcStatus.dwWin32ExitCode = 0;
		g_svcStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_svcHandle, &g_svcStatus) == FALSE) {
			p("SetServiceStatus failed", GetLastError());
		}

		SetEvent(g_svcEvent);
		break;

	default:
		break;
	}
}

DWORD WINAPI service_handler(LPVOID) {
	debugp p(__FUNCTION__);
	hfst_init();

	PSECURITY_DESCRIPTOR psd;
	if (!ConvertStringSecurityDescriptorToSecurityDescriptorA("D:(A;;GA;;;WD)(A;;GA;;;AN)", SDDL_REVISION_1, &psd, nullptr)) {
		p("ConvertStringSecurityDescriptorToSecurityDescriptor failed", GetLastError());
	}

	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES) };
	sa.lpSecurityDescriptor = psd;
	sa.bInheritHandle = FALSE;

	while (WaitForSingleObject(g_svcEvent, 0) != WAIT_OBJECT_0) {
		Sleep(1000);
		HANDLE pipe = CreateNamedPipeA(conf["PIPE"].c_str(), PIPE_ACCESS_DUPLEX, PIPE_WAIT, 1, 1024, 1024, 7 * 1000, &sa);

		if (pipe == INVALID_HANDLE_VALUE) {
			p("CreateNamedPipeA failed", GetLastError());
			continue;
		}

		p("Waiting for connection");
		ConnectNamedPipe(pipe, nullptr);

		p("Reading pipe");
		DWORD bytes = 0;
		DWORD bytes_read = 0;
		cbuffer.resize(1);
		if (!ReadFile(pipe, &cbuffer[0], 1, &bytes_read, 0) || bytes_read != 1) {
			p("ReadFile(pipe) 1", GetLastError());
			continue;
		}
		if (!PeekNamedPipe(pipe, 0, 0, 0, &bytes, 0)) {
			p("PeekNamedPipe(pipe)", GetLastError());
			continue;
		}
		if (bytes) {
			cbuffer.resize(1 + bytes);
			if (!ReadFile(pipe, &cbuffer[1], bytes, &bytes_read, 0) || bytes != bytes_read) {
				p("ReadFile(pipe) 2", GetLastError());
				continue;
			}
		}

		p("Writing HFST", cbuffer);
		bytes = 0;
		bytes_read = 0;
		if (!WriteFile(g_hChildStd_IN_Wr, cbuffer.c_str(), cbuffer.size(), &bytes, 0) || bytes != cbuffer.size()) {
			p("WriteFile(hfst)", GetLastError());
			return false;
		}
		p("Reading HFST");
		cbuffer.resize(1);
		if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[0], 1, &bytes_read, 0) || bytes_read != 1) {
			p("ReadFile(hfst) 1", GetLastError());
			continue;
		}
		if (!PeekNamedPipe(g_hChildStd_OUT_Rd, 0, 0, 0, &bytes, 0)) {
			p("PeekNamedPipe(hfst)", GetLastError());
			continue;
		}
		if (bytes) {
			cbuffer.resize(1 + bytes);
			if (!ReadFile(g_hChildStd_OUT_Rd, &cbuffer[1], bytes, &bytes_read, 0) || bytes != bytes_read) {
				p("ReadFile(hfst) 2", GetLastError());
				continue;
			}
		}

		p("Writing pipe", cbuffer);
		bytes = 0;
		bytes_read = 0;
		if (!WriteFile(pipe, cbuffer.c_str(), cbuffer.size(), &bytes, 0) || bytes != cbuffer.size()) {
			p("WriteFile(pipe)", GetLastError());
			continue;
		}

		CloseHandle(pipe);
	}

	hfst_terminate();
	return ERROR_SUCCESS;
}

void WINAPI service_main(DWORD, LPSTR*) {
	debugp p(__FUNCTION__);

	g_svcHandle = RegisterServiceCtrlHandlerA(conf["SERVICE_NAME"].c_str(), service_control);

	if (g_svcHandle == nullptr) {
		p("RegisterServiceCtrlHandler failed", GetLastError());
		return;
	}

	ZeroMemory(&g_svcStatus, sizeof(g_svcStatus));
	g_svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_svcStatus.dwCurrentState = SERVICE_START_PENDING;
	g_svcStatus.dwControlsAccepted = 0;
	g_svcStatus.dwWin32ExitCode = 0;
	g_svcStatus.dwServiceSpecificExitCode = 0;
	g_svcStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_svcHandle, &g_svcStatus) == FALSE) {
		p("SetServiceStatus failed", GetLastError());
	}

	g_svcEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	if (g_svcEvent == nullptr) {
		p("CreateEvent failed", GetLastError());

		g_svcStatus.dwCurrentState = SERVICE_STOPPED;
		g_svcStatus.dwControlsAccepted = 0;
		g_svcStatus.dwWin32ExitCode = GetLastError();
		g_svcStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_svcHandle, &g_svcStatus) == FALSE) {
			p("SetServiceStatus failed", GetLastError());
		}
		return;
	}

	g_svcStatus.dwCurrentState = SERVICE_RUNNING;
	g_svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_svcStatus.dwWin32ExitCode = 0;
	g_svcStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_svcHandle, &g_svcStatus) == FALSE) {
		p("SetServiceStatus failed", GetLastError());
	}

	HANDLE thread = CreateThread(nullptr, 0, service_handler, nullptr, 0, nullptr);

	WaitForSingleObject(thread, INFINITE);
	CloseHandle(g_svcEvent);

	g_svcStatus.dwCurrentState = SERVICE_STOPPED;
	g_svcStatus.dwControlsAccepted = 0;
	g_svcStatus.dwWin32ExitCode = 0;
	g_svcStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_svcHandle, &g_svcStatus) == FALSE) {
		p("SetServiceStatus failed", GetLastError());
	}
}

int main() {
	debugp p(__FUNCTION__);

	if (!read_conf(conf)) {
		return -__LINE__;
	}

	SERVICE_TABLE_ENTRYA svc_table[] = {
		{ const_cast<char*>(conf["SERVICE_NAME"].c_str()), (LPSERVICE_MAIN_FUNCTIONA)service_main },
		{ nullptr, nullptr }
	};

	if (StartServiceCtrlDispatcherA(svc_table) == FALSE) {
		p("StartServiceCtrlDispatcher failed", GetLastError());
		return GetLastError();
	}
}
