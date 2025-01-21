// SampleWindbgScript.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "../Util/LinkedList.h"
#include "CharacterManager.h"
#include <conio.h>

CCharacterManager* g_pCharacterManager = nullptr;

int main()
{
#ifdef _DEBUG
	int	flag = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flag);
#endif

	srand(1);

	const WCHAR* wchNames[] = {L"Kasumi", L"Ayane", L"Hitomi", L"Leifang", L"Tina", L"Kokoro"};
	g_pCharacterManager = new CCharacterManager;

	const DWORD CHR_COUNT = 1000;
	for (DWORD i = 0; i < CHR_COUNT; i++)
	{
		int iHP = rand() % 254 + 1;
		int iMP = rand() % 254 + 1;
		DWORD dwNameCount = (DWORD)_countof(wchNames);
		DWORD dwNameIndex = rand() % dwNameCount;
		CHARACTER* pChr = g_pCharacterManager->CreateCharacter(i, wchNames[dwNameIndex], iHP, iMP);
	}

	DWORD dwFrameCount = 0;
	while (1)
	{
		if (GetAsyncKeyState(VK_SPACE))
		{
			g_pCharacterManager->OccurBug();
		}
		else if (GetAsyncKeyState(VK_ESCAPE))
		{
			break;
		}
		g_pCharacterManager->Process();
		Sleep(33);
		wprintf_s(L"Frame Increase...[%u]\n", dwFrameCount);
		dwFrameCount++;
	}

	if (g_pCharacterManager)
	{
		g_pCharacterManager->DeleteAllCharacters();

		delete g_pCharacterManager;
		g_pCharacterManager = nullptr;
	}
	
#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif
}
