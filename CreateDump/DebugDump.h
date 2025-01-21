#pragma once



enum DUMP_TYPE
{
	DUMP_TYPE_MINI,
	DUMP_TYPE_FULL,
	DUMP_TYPE_COUNT
};

void InitMiniDump(const WCHAR* wchAppPath, const WCHAR* wchAppName, DWORD dwBuildNumber);
void DumpMiniDump(EXCEPTION_POINTERS *pE, DUMP_TYPE dumpType);
LONG WINAPI ErrorDumpHandler(EXCEPTION_POINTERS *pE);
void SetDumpTypeToMini();
void SetDumpTypeToFull();

//#define __FULL_DUMP__

//void	InitDBGInfo();
//void	UninitDBGInfo();
