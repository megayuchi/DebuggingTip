// CreateDump.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <Windows.h>
#include <Shlwapi.h>
#include "DebugDump.h"

#pragma comment(lib, "Shlwapi.lib")

int CRT_AllocHook(int allocType, void *userData, size_t size,
				  int blockType, long requestNumber,
				  const unsigned char *filename, int lineNumber)
{
	if (size == 0x400)	// 1024
	{
		int a = 0;
	}
	return TRUE;
}


int main()
{
    BOOL bIsDebuggerPresent = IsDebuggerPresent() != 0;
	
#ifdef _DEBUG
	int	Flags = _CRTDBG_ALLOC_MEM_DF |_CRTDBG_LEAK_CHECK_DF;
	_CrtSetDbgFlag(Flags);

	// 
	_CrtSetAllocHook(CRT_AllocHook);

#endif
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

	WCHAR wchExePath[_MAX_PATH] = {};
	GetModuleFileName(nullptr, wchExePath, (DWORD)_countof(wchExePath));
	PathRemoveFileSpec(wchExePath);

	InitMiniDump(wchExePath, L"CreateDump", 1);


	if (!bIsDebuggerPresent)
	{
		SetUnhandledExceptionFilter(ErrorDumpHandler);
	}

	// Ű���� �Է��غ�
	INPUT_RECORD	irBuffer;
	memset(&irBuffer, 0, sizeof(INPUT_RECORD));
	DWORD	dwResult;
   
	BOOL bLoop = TRUE;
	while (bLoop)
	{

		ReadConsoleInput(hIn, &irBuffer, 1, &dwResult);

		if (irBuffer.EventType == KEY_EVENT)
		{

			if (irBuffer.Event.KeyEvent.bKeyDown)
			{
				if (irBuffer.Event.KeyEvent.wVirtualKeyCode == VK_F1)
				{
				}
				if (irBuffer.Event.KeyEvent.wVirtualKeyCode == VK_F10)
				{
					wprintf_s(L"trying overrun memory...\n");
					Sleep(250);

					// heap corruption�� ���ؼ��� SetUnhandledExceptionFilter()�۵��� ������� �ʴ´�.
					// �� ������ ���ؼ��� dump�� ����� �ʹٸ� aedebug�������� ����Ÿ� ��� ȣ���Ű�ų� Dr.Watson�� ����Ѵ�.
					char* p = new char[1024];
					//memset(p, 0, 1024 + 4);	// CRT Heap���� ���������� �߰��� ��Ÿ�����͸� �ջ�� ���. __debugbreak()�� ȣ��ǰ� SetUnhandledExceptionFilter()���� ���ܰ� ����.
					memset(p, 0, 1024 + 16);	// NtHeap�� ��Ÿ�����ͱ��� �ջ��Ų ���. SetUnhandledExceptionFilter()���� ���ܸ� ���� �� ����. dump�� �߷��� �ܺ� ���� ���.
					delete[] p;
					p = nullptr;

					
				}
				if (irBuffer.Event.KeyEvent.wVirtualKeyCode == VK_F12)
				{
					wprintf_s(L"debugbreak()...\n");
					Sleep(250);
					__debugbreak();
				}

				if (irBuffer.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE)
				{
					wprintf_s(L"Shutdown...\n");
					bLoop = FALSE;
				}
			}
		}
	}

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
