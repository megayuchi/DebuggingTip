// TestGameFramework.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "stdafx.h"
#include <Windows.h>
#include <iostream>
#include <conio.h>
#include "GameObject.h"
#include "Game.h"

int main()
{
#ifdef _DEBUG
	int	flag = _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(flag);
#endif
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

	InitializeGame(hOut);

	while (1)
	{
		BOOL	bExit = FALSE;
		GameLoop(&bExit);
		if (bExit)
		{
			break;
		}
	}
	CleanupGame();

	//WriteConsoleOutput
	wprintf_s(L"The application has been terminated. Press any key.\n");
	_getch();

#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif

}
