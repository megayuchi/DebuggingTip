// HeapCorruption.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <Windows.h>
#include <conio.h>
#include <stdio.h>
#include <crtdbg.h>

struct ITEM
{
	DWORD dwID;
	WCHAR wchName[32];
};

int CRT_AllocHook(int allocType, void *userData, size_t size,
				  int blockType, long requestNumber,
				  const unsigned char *filename, int lineNumber)
{
	if (size == 6800)
	{
		int a = 0;
	}
	return TRUE;
}
int main()
{
	
#ifdef _DEBUG
	int	Flags = _CRTDBG_ALLOC_MEM_DF |_CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(Flags);

	_CrtSetAllocHook(CRT_AllocHook);
#endif
	const WCHAR* wchNameList[] = { L"Hitomi", L"Kasumi", L"Leifang", L"Ayane" };

	HANDLE pHeapList[64] = {};
	//DWORD dwHeapCount = GetProcessHeaps((DWORD)_countof(pHeapList), pHeapList);
	//wprintf_s(L"Cureent Heaps count: %u, Press any key\n", dwHeapCount);
	//_getch();

	HANDLE hHeap = nullptr;
	char* p = nullptr;

	const DWORD AllocSize = 6800;
	p = (char*)HeapAlloc(GetProcessHeap(), 0, AllocSize);
	memset(p, 0xff, AllocSize);
	HeapFree(GetProcessHeap(), 0, p);
	p = nullptr;
	//wprintf_s(L"memset\n");
	//memset(p, 0, 6804);

	//wprintf_s(L"HeapFree\n");
	//HeapFree(hHeap, 0, p);
	//p = nullptr;

	//wprintf_s(L"HeapAlloc(%u), Press any key\n", AllocSize);
	//_getch();
	//__debugbreak();
	ITEM* pItemList = (ITEM*)malloc(sizeof(ITEM) * 100);
	memset(pItemList, 0xff, sizeof(ITEM) * 100);
	
	for (DWORD i = 0; i < 100; i++)
	{
		pItemList[i].dwID = (DWORD)rand() % 256;
		DWORD dwNameIndex = (DWORD)rand() % (DWORD)_countof(wchNameList);
		wcscpy_s(pItemList[i].wchName, wchNameList[dwNameIndex]);
	}

	DWORD dwHeapCount = GetProcessHeaps((DWORD)_countof(pHeapList), pHeapList);
	wprintf_s(L"Cureent Heaps count: %u, Press any key\n", dwHeapCount);
	for (DWORD i = 0; i < dwHeapCount; i++)
	{
		wprintf_s(L"Heap[%u]:%p\n", i, pHeapList[i]);
	}
	_getch();

	// heap corruption!!!
	pItemList[100].dwID = 512;
	wcscpy_s(pItemList[100].wchName, wchNameList[0]);

	if (pItemList)
	{
		free(pItemList);
		pItemList = nullptr;
	}
	if (hHeap)
	{
		HeapFree(hHeap, 0, p);
		HeapDestroy(hHeap);
	}

	wprintf_s(L"Press any key.\n");
    _getch();
#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
