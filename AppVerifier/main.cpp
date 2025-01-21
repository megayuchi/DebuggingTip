// AppVerifier.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <memory.h>
#include <conio.h>

int main()
{
    
#ifdef _DEBUG
	int	Flags = _CRTDBG_ALLOC_MEM_DF |_CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(Flags);
#endif

    DWORD* p0 = new DWORD[1024];
    DWORD* p1 = new DWORD[512];
    DWORD* p2 = new DWORD[256];

    p1[512] = 0xff;
   
    delete[] p0;
    delete[] p1;
    delete[] p2;


    
#ifdef _DEBUG
	_ASSERT(_CrtCheckMemory());
#endif
    wprintf_s(L"Press any key.\n");
    _getch();
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
