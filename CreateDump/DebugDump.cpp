#include "pch.h"
#include <Windows.h>
#include <time.h>
#include <stdio.h>
#include <IPHlpApi.h>
#include "DebugDump.h"

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "IPHlpApi.lib")

#define __FULL_DUMP__

#ifdef __FULL_DUMP__
#define		DEFAULT_DUMP_TYPE	DUMP_TYPE_FULL
#else
#define		DEFAULT_DUMP_TYPE	DUMP_TYPE_MINI
#endif


BOOL	g_bWriteSystemInfo = FALSE;
WCHAR	g_wchDmpFileName[_MAX_PATH] = {};
WCHAR	g_wchSysInfoFileName[_MAX_PATH] = {};
DUMP_TYPE	g_curDumpType = DEFAULT_DUMP_TYPE;

BOOL GetAdapterInfo(WCHAR* wchOutMAC, DWORD dwMaxBufferCount);

BOOL GetAdapterInfo(WCHAR* wchOutMAC, DWORD dwMaxBufferCount)
{
	BOOL	bResult = FALSE;
	DWORD size = 0;
	IP_ADAPTER_INFO* pInfo = nullptr;

	int result = GetAdaptersInfo(pInfo, &size);
	if (result == ERROR_BUFFER_OVERFLOW)    // GetAdaptersInfo가 메모리가 부족하면 재 할당하고 재호출
	{
		pInfo = (PIP_ADAPTER_INFO)malloc(size);
		GetAdaptersInfo(pInfo, &size);
	}

	if (!pInfo)
		return FALSE;

	swprintf_s(wchOutMAC, dwMaxBufferCount, L"%02X%02X%02X%02X%02X%02X", pInfo->Address[0], pInfo->Address[1], pInfo->Address[2], pInfo->Address[3], pInfo->Address[4], pInfo->Address[5]);

	free(pInfo);
	bResult = TRUE;

lb_return:
	return bResult;
}


void InitMiniDump(const WCHAR* wchAppPath, const WCHAR* wchAppName, DWORD dwBuildNumber)
{
	WCHAR wchMAC[32];
	GetAdapterInfo(wchMAC, (DWORD)_countof(wchMAC));

	const WCHAR*	wchConfig = L"";
#ifdef _DEBUG
	wchConfig = L"debug";
#else
	wchConfig = L"release";
#endif
	const WCHAR* wchArch = L"";
#if defined(_M_ARM64EC)
	wchArch = L"arm64ec";
#elif defined(_M_ARM64)
	wchArch = L"arm64";
#elif defined(_M_AMD64)
	wchArch = L"x64";
#elif defined(_M_IX86)
	wchArch = L"x86";
#endif
	swprintf_s(g_wchDmpFileName, L"%s\\%s_%s_%s_%03d_%s", wchAppPath,wchAppName, wchArch, wchConfig, dwBuildNumber, wchMAC);
}

LONG WINAPI ErrorDumpHandler(EXCEPTION_POINTERS *pE)
{
	wprintf_s(L"ErrorDumpHandler");
	Sleep(200);

	LONG	lResult = EXCEPTION_EXECUTE_HANDLER;
	CONTEXT	context = *pE->ContextRecord;

	DumpMiniDump(pE, g_curDumpType);

	*pE->ContextRecord = context;

	return lResult;
}

void DumpMiniDump(EXCEPTION_POINTERS *pE, DUMP_TYPE dumpType)
{
	MINIDUMP_TYPE	miniDumpType = MiniDumpNormal;

	//SetUnhandledExceptionFilter(nullptr);
	//__debugbreak();

	if (DUMP_TYPE_FULL == dumpType)
	{
		// CUDA관련 dll이 로드된 상태에서 MiniDumpWriteDump()함수에 MiniDumpWithFullMemory플래그를 주면 실패한다.
		// OS의 태스크매니저에서 덤프를 만드는 경우는 성공한다.
		// MiniDumpIgnoreInaccessibleMemory플래그를 추가로 설정하면 일단은 이 문제를 피해갈수 있다. cuda unified memory쪽이랑 충돌하는것 같다. cudaMallocMangaed()로 할당한
		// 메모리가 page faule를 일으키기 때문에 그쪽 메모리를 억세스할때 문제가 생기는걸로 보인다.
		miniDumpType = MINIDUMP_TYPE(MiniDumpWithFullMemory | MiniDumpIgnoreInaccessibleMemory);
	}
	else
	{
		//miniDumpType = MiniDumpWithDataSegs;
		miniDumpType = MINIDUMP_TYPE(MiniDumpWithDataSegs | MiniDumpWithCodeSegs | MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory);
	}



	__time64_t long_time;
	_time64(&long_time);           // Get time as 64-bit integer.

	tm	local_time;
	memset(&local_time, 0, sizeof(local_time));
	_localtime64_s(&local_time, &long_time);

	WCHAR	wchDumpTime[32];
	swprintf_s(wchDumpTime, L"%02d%02d%02d%02d%02d", local_time.tm_mon + 1, local_time.tm_mday, local_time.tm_hour, local_time.tm_min, local_time.tm_sec);

	WCHAR wchDumpFileName[_MAX_PATH] = {};
	swprintf_s(wchDumpFileName, L"%s_%s.dmp", g_wchDmpFileName, wchDumpTime);

	HANDLE hFile = CreateFileW(wchDumpFileName,
							   FILE_WRITE_DATA,
							   0,
							   nullptr,
							   CREATE_ALWAYS,
							   0,
							   nullptr);


	MINIDUMP_EXCEPTION_INFORMATION	exceptionInfo = {};
	MINIDUMP_EXCEPTION_INFORMATION*	pExceptionInfo = nullptr;

	HANDLE	hProcess = GetCurrentProcess();
	DWORD	dwProcessID = GetCurrentProcessId();
	DWORD	dwThreadID = GetCurrentThreadId();

	if (pE)
	{
		memset(&exceptionInfo, 0, sizeof(exceptionInfo));
		exceptionInfo.ThreadId = dwThreadID;
		exceptionInfo.ExceptionPointers = pE;
		exceptionInfo.ClientPointers = TRUE;
		pExceptionInfo = &exceptionInfo;
	}
	BOOL bDumpResult = FALSE;
	DWORD dwDumpError = 0;
	if (hFile)
	{
		exceptionInfo.ClientPointers = TRUE;

		bDumpResult = MiniDumpWriteDump(hProcess,
										dwProcessID,
										hFile,
										miniDumpType,
										pExceptionInfo,
										nullptr,
										nullptr
		);
		if (!bDumpResult)
		{
			dwDumpError = GetLastError();
		}

		CloseHandle(hFile);
	}
}
void SetDumpTypeToFull()
{
	g_curDumpType = DUMP_TYPE_FULL;
}
void SetDumpTypeToMini()
{
	g_curDumpType = DUMP_TYPE_MINI;
}

//This method can be used to have the current process be able to call MiniDumpWriteDump
