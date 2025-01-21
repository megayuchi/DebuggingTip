// CrtHeapDiff.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <Windows.h>
#include <stdio.h>
#include <conio.h>


int main()
{
#ifdef _DEBUG
	int	flags = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	//int	flags = _CRTDBG_ALLOC_MEM_DF;
	_CrtSetDbgFlag(flags);
#endif

	// take a snapshot
#ifdef _DEBUG
	_CrtMemState s_prv = {};
	_CrtMemCheckpoint(&s_prv);
#endif

	// alloc memory
	char* p0 = new char[100];
	char* p1 = new char[100];


	// take a snapshot
#ifdef _DEBUG
	_CrtMemState s_cur = {};
	_CrtMemCheckpoint(&s_cur);
#endif


	// diff 2 snapshots
#ifdef _DEBUG
	_CrtMemState s_diff = {};
	if (_CrtMemDifference(&s_diff, &s_prv, &s_cur))
	{
		_CrtMemDumpStatistics(&s_diff);
	}
#endif


#ifdef _DEBUG

	// check memory leaks
	int	leak = _CrtDumpMemoryLeaks();
	_CrtMemDumpAllObjectsSince(nullptr);

	// heap validation
	_ASSERT(_CrtCheckMemory());
#endif

	wprintf(L"Press any key.\n");
	_getch();
}
