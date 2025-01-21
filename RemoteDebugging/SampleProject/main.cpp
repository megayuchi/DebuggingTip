// Local.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <Windows.h>

int main()
{
    wprintf_s(L"Begin Remote Debugging.\n");

    for (DWORD i = 0; i < 10; i++)
    {
        wprintf_s(L"Test:[%u]\n", i);
    }
    wprintf_s(L"End Remote Debugging.\n");
    wprintf_s(L"Press any key...\n");
    _getch();
}
