#include "stdafx.h"
#include "typedef.h"
#include "GameObject.h"
#include "Game.h"
#include "StaticData.h"
#include "game_typedef.h"

int	g_iCursorPosX = 0;
int g_iCursorPosY = 0;

int g_iScreenWidth = 0;
int g_iScreenHeight = 0;

WCHAR* g_pBackBuffer = nullptr;
WCHAR* g_pBackground = nullptr;
WCHAR* g_pBackgroundLineBuffer = nullptr;

BOOL g_bLeftKeyPressed = FALSE;
BOOL g_bUpKeyPressed = FALSE;
BOOL g_bRightKeyPressed = FALSE;
BOOL g_bDownKeyPressed = FALSE;
BOOL g_bSpaceKeyPressed = FALSE;
DWORD g_dwCurScore = 0;

HANDLE g_hOut = nullptr;
DWORD	g_dwCurFPS = 0;
const DWORD g_dwGameFPS = 60;
const DWORD g_dwTicksPerGameFame = 1000 / g_dwGameFPS;
ULONGLONG g_PrvGameFrameTick = 0;

FLIGHT_OBJECT*	g_pPlayer = nullptr;


FLIGHT_OBJECT*	g_ppAmmoList[MAX_AMMO_NUM] = {};
DWORD g_dwCurAmmoNum = 0;

FLIGHT_OBJECT*	g_ppEnemyList[MAX_ENEMY_NUM] = {};
DWORD g_dwCurEnemiesNum = 0;

// control game status
void OnGameFrame(ULONGLONG CurTick);
void ProcessEnemies();
void OnHitEnemy(FLIGHT_OBJECT* pEnemy, ULONGLONG CurTick);
DWORD AddScore(DWORD dwAddval);
void FillEnemies();
void MoveEnemies();
BOOL IsCollisionFlightObjectVsFlightObject(const FLIGHT_OBJECT* pObj0, const FLIGHT_OBJECT* pObj1);
BOOL ProcessCollisionAmmoVsEnemies(FLIGHT_OBJECT* pAmmo, ULONGLONG CurTick);
void ProcessCollision(ULONGLONG CurTick);
void DeleteDestroyedEnemies(ULONGLONG CurTick);
void ShootFromPlayer();
void DeleteAllAmmos();
void UpdateBackground();

// display
void ClearScreen();
void DrawCursor();
void Blt();
void ClearBackBuffer();
void UpdateFPS(DWORD dwFPS);
void DrawFPS(int x, int y);
void DrawScore(int x, int y);
void DrawScene();
void DrawFlightObject(FLIGHT_OBJECT* pFighter, int x, int y);

// keyboard input
void OnKeyDown_Left();
void OnKeyUp_Left();
void OnKeyDown_Up();
void OnKeyUp_Up();
void OnKeyDown_Right();
void OnKeyUp_Right();
void OnKeyDown_Down();
void OnKeyUp_Down();
void OnKeyDown_Space();
void OnKeyUp_Space();
BOOL ProcessInput(BOOL* pbOutExit);

BOOL InitializeGame(HANDLE hOut)
{
	srand(GetTickCount());

	g_hOut = hOut;
	CONSOLE_SCREEN_BUFFER_INFO	info = {};
	GetConsoleScreenBufferInfo(hOut, &info);
	g_iScreenWidth = (int)info.dwSize.X;
	g_iScreenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;


	g_pBackBuffer = new WCHAR[g_iScreenWidth * g_iScreenHeight];
	for (DWORD i = 0; i < (DWORD)(g_iScreenWidth * g_iScreenHeight); i++)
	{
		g_pBackBuffer[i] = L'.';
	}
	g_pBackground = new WCHAR[g_iScreenWidth * g_iScreenHeight];
	g_pBackgroundLineBuffer = new WCHAR[g_iScreenWidth];
	for (DWORD i = 0; i < (DWORD)(g_iScreenWidth * g_iScreenHeight); i++)
	{
		g_pBackground[i] = L' ';
	}
	//DWORD	dwStartNum = (g_iScreenWidth / 16) * (g_iScreenHeight / 8);
	DWORD	dwStartNum = 36;
	for (DWORD i = 0; i < dwStartNum; i++)
	{
		int x = rand() % (g_iScreenWidth / 8) * 8;
		int y = rand() % (g_iScreenHeight / 4) * 4;


		if (x >= g_iScreenWidth)
			__debugbreak();

		if (y >= g_iScreenHeight)
			__debugbreak();

		g_pBackground[x + y * g_iScreenWidth] = L'.';
	}

	COORD	pos = {};
	DWORD	dwWrittenChrs = 0;
	FillConsoleOutputCharacter(hOut, L'.', (DWORD)(g_iScreenWidth * g_iScreenHeight), pos, &dwWrittenChrs);

	g_pPlayer = CreatePlayer();


	return TRUE;
}
void GameLoop(BOOL* pbOutExit)
{
	ProcessInput(pbOutExit);

	ULONGLONG CurTick = GetTickCount64();
	if ((DWORD)(CurTick - g_PrvGameFrameTick) > g_dwTicksPerGameFame)
	{
		OnGameFrame(CurTick);
		g_PrvGameFrameTick = CurTick;
	}
	DrawScene();
}


BOOL ProcessInput(BOOL* pbOutExit)
{
	BOOL	bUpdated = FALSE;

	if (GetAsyncKeyState(VK_LEFT))
	{
		if (!g_bLeftKeyPressed)
		{
			OnKeyDown_Left();
			bUpdated = TRUE;
		}
		g_bLeftKeyPressed = TRUE;
	}
	else
	{
		if (g_bLeftKeyPressed)
		{
			OnKeyUp_Left();
			bUpdated = TRUE;
		}
		g_bLeftKeyPressed = FALSE;
	}
	if (GetAsyncKeyState(VK_RIGHT))
	{
		if (!g_bRightKeyPressed)
		{
			OnKeyDown_Right();
			bUpdated = TRUE;
		}
		g_bRightKeyPressed = TRUE;
	}
	else
	{
		if (g_bRightKeyPressed)
		{
			OnKeyUp_Right();
			bUpdated = TRUE;
		}
		g_bRightKeyPressed = FALSE;
	}

	if (GetAsyncKeyState(VK_UP))
	{
		if (!g_bUpKeyPressed)
		{
			OnKeyDown_Up();
			bUpdated = TRUE;
		}
		g_bUpKeyPressed = TRUE;
	}
	else
	{
		if (g_bUpKeyPressed)
		{
			OnKeyUp_Up();
			bUpdated = TRUE;
		}
		g_bUpKeyPressed = FALSE;
	}
	if (GetAsyncKeyState(VK_DOWN))
	{
		if (!g_bDownKeyPressed)
		{
			OnKeyDown_Down();
			bUpdated = TRUE;
		}
		g_bDownKeyPressed = TRUE;
	}
	else
	{
		if (g_bDownKeyPressed)
		{
			OnKeyUp_Down();
			bUpdated = TRUE;
		}
		g_bDownKeyPressed = FALSE;
	}
	if (GetAsyncKeyState(VK_SPACE))
	{
		if (!g_bSpaceKeyPressed)
		{
			OnKeyDown_Space();
			bUpdated = TRUE;
		}
		g_bSpaceKeyPressed = TRUE;
	}
	else
	{
		if (g_bSpaceKeyPressed)
		{
			OnKeyUp_Space();
			bUpdated = TRUE;
		}
		g_bSpaceKeyPressed = FALSE;
	}
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		*pbOutExit = TRUE;
		bUpdated = TRUE;
	}
	return bUpdated;
}
void DrawScene()
{
	ClearBackBuffer();

	// Draw player
	DrawFlightObject(g_pPlayer, g_pPlayer->x, g_pPlayer->y);

	// Draw Enemies
	for (DWORD i = 0; i < g_dwCurEnemiesNum; i++)
	{
		DrawFlightObject(g_ppEnemyList[i], g_ppEnemyList[i]->x, g_ppEnemyList[i]->y);
	}

	// Draw ammos
	for (DWORD i = 0; i < g_dwCurAmmoNum; i++)
	{
		DrawFlightObject(g_ppAmmoList[i], g_ppAmmoList[i]->x, g_ppAmmoList[i]->y);
	}
	DrawFPS(1, 1);
	DrawScore(g_iScreenWidth - 16, 1);

	Blt();
}
void DrawFlightObject(FLIGHT_OBJECT* pFighter, int x, int y)
{
	int dest_start_x = max(x, 0);
	int dest_start_y = max(y, 0);
	dest_start_x = min(dest_start_x, g_iScreenWidth - 1);
	dest_start_y = min(dest_start_y, g_iScreenHeight - 1);

	int dest_end_x = max(x + pFighter->Width, 0);
	int dest_end_y = max(y + pFighter->Height, 0);
	dest_end_x = min(dest_end_x, g_iScreenWidth);
	dest_end_y = min(dest_end_y, g_iScreenHeight);

	int	width = dest_end_x - dest_start_x;
	int	height = dest_end_y - dest_start_y;

	if (width <= 0 || height <= 0)
		return;

	int src_start_x = dest_start_x - x;
	int src_start_y = dest_start_y - y;

	//if (x > g_iScreenWidth - 1)
	//{
	//	x = g_iScreenWidth - 1;
	//}
	//if (y > g_iScreenHeight - 1)
	//{
	//	y = g_iScreenHeight - 1;
	//}
	//y = 10;
	//g_pBackBuffer[x + y * g_iScreenWidth] = L'x';
	//g_pBackBuffer[dest_start_x + dest_start_y * g_iScreenWidth] = L'x';


	WCHAR* pSrc = pFighter->pBits + src_start_x + src_start_y * pFighter->Width;
	WCHAR* pDest = g_pBackBuffer + dest_start_x + dest_start_y * g_iScreenWidth;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			*pDest = *pSrc;
			pSrc++;
			pDest++;
		}
		pSrc -= width;
		pSrc += pFighter->Width;
		pDest -= width;
		pDest += g_iScreenWidth;
	}
}
void OnGameFrame(ULONGLONG CurTick)
{
	if (g_bLeftKeyPressed)
	{
		g_pPlayer->x--;
		if (g_pPlayer->x < 0)
			g_pPlayer->x = 0;
	}
	if (g_bRightKeyPressed)
	{
		g_pPlayer->x++;
		if (g_pPlayer->x > g_iScreenWidth - g_pPlayer->Width)
			g_pPlayer->x = g_iScreenWidth - g_pPlayer->Width;
	}
	if (g_bUpKeyPressed)
	{
		g_pPlayer->y--;
		if (g_pPlayer->y < 0)
			g_pPlayer->y = 0;
	}
	if (g_bDownKeyPressed)
	{
		g_pPlayer->y++;
		if (g_pPlayer->y > g_iScreenHeight - g_pPlayer->Height)
			g_pPlayer->y = g_iScreenHeight - g_pPlayer->Height;
	}

	// 사망한 적들 제거
	DeleteDestroyedEnemies(CurTick);

	ProcessCollision(CurTick);

	// 탄환이 맵 끝에 도달하면 자동 파괴
	DWORD	dwIndex = 0;
	while (dwIndex < g_dwCurAmmoNum)
	{
		FLIGHT_OBJECT*	pAmmo = g_ppAmmoList[dwIndex];
		if (pAmmo->y < 0)
		{
			DeleteFlightObject(pAmmo);
			g_dwCurAmmoNum--;
			g_ppAmmoList[dwIndex] = g_ppAmmoList[g_dwCurAmmoNum];
			g_ppAmmoList[g_dwCurAmmoNum] = nullptr;
		}
		else
		{
			pAmmo->y--;
			dwIndex++;
		}
	}

	ProcessEnemies();

	UpdateBackground();
}
void UpdateBackground()
{
	static ULONGLONG PrvUpdateTick = 0;
	ULONGLONG CurTick = GetTickCount64();

	if (CurTick - PrvUpdateTick < 1000)
		return;
	
	PrvUpdateTick = CurTick;

	DWORD	dwTotalPixels = (DWORD)(g_iScreenWidth * g_iScreenHeight);
	
	
	WCHAR* pSrc = g_pBackground + (g_iScreenHeight - 1) * g_iScreenWidth;
	WCHAR* pDest = g_pBackgroundLineBuffer;
	
	for (int y = 0; y < g_iScreenHeight; y++)
	{
		memcpy(pDest, pSrc, sizeof(WCHAR) * g_iScreenWidth);
		pDest = pSrc;
		pSrc -= g_iScreenWidth;
	}
	memcpy(g_pBackground, g_pBackgroundLineBuffer, sizeof(WCHAR) * g_iScreenWidth);
	//memcpy(g_pBackground, g_pBackground + g_iScreenWidth, sizeof(WCHAR) * (dwTotalPixels - (DWORD)g_iScreenWidth));
	//memcpy(g_pBackground + (dwTotalPixels - (DWORD)g_iScreenWidth), g_pBackgroundLineBuffer, sizeof(WCHAR) * g_iScreenWidth);
}
void DeleteDestroyedEnemies(ULONGLONG CurTick)
{
	DWORD	dwIndex = 0;
	while (dwIndex < g_dwCurEnemiesNum)
	{
		FLIGHT_OBJECT*	pEnemy = g_ppEnemyList[dwIndex];
		if (pEnemy->status == FLIGHT_OBJECT_STATUS_DEAD && CurTick - pEnemy->DeadTick > DEAD_STATUS_WAIT_TICK)
		{
			DeleteFlightObject(pEnemy);
			g_dwCurEnemiesNum--;
			g_ppEnemyList[dwIndex] = g_ppEnemyList[g_dwCurEnemiesNum];
			g_ppEnemyList[g_dwCurEnemiesNum] = nullptr;
		}
		else
		{
			dwIndex++;
		}
	}
}
void ProcessCollision(ULONGLONG CurTick)
{
	DWORD	dwIndex = 0;
	while (dwIndex < g_dwCurAmmoNum)
	{
		FLIGHT_OBJECT*	pAmmo = g_ppAmmoList[dwIndex];
		if (ProcessCollisionAmmoVsEnemies(pAmmo, CurTick))
		{
			DeleteFlightObject(pAmmo);
			g_dwCurAmmoNum--;
			g_ppAmmoList[dwIndex] = g_ppAmmoList[g_dwCurAmmoNum];
			g_ppAmmoList[g_dwCurAmmoNum] = nullptr;
		}
		else
		{
			dwIndex++;
		}
	}
}

BOOL IsCollisionRectVsRect(const INT_VECTOR2* pv3MinA, const INT_VECTOR2* pv3MaxA, const INT_VECTOR2* pv3MinB, const INT_VECTOR2* pv3MaxB)
{
	const int*	a_min = &pv3MinA->x;
	const int*	a_max = &pv3MaxA->x;
	const int*	b_min = &pv3MinB->x;
	const int*	b_max = &pv3MaxB->x;

	for (DWORD i = 0; i < 2; i++)
	{
		if (a_min[i] > b_max[i] || a_max[i] < b_min[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}
BOOL IsCollisionFlightObjectVsFlightObject(const FLIGHT_OBJECT* pObj0, const FLIGHT_OBJECT* pObj1)
{
	BOOL bResult = FALSE;

	if (pObj0->status != FLIGHT_OBJECT_STATUS_ALIVE || pObj1->status != FLIGHT_OBJECT_STATUS_ALIVE)
	{
		return FALSE;
	}
	INT_RECT2	objRect0 =
	{
		pObj0->x, pObj0->y,
		pObj0->x + pObj0->Width, pObj0->y + pObj0->Height
	};
	INT_RECT2	objRect1 =
	{
		pObj1->x, pObj1->y,
		pObj1->x + pObj1->Width, pObj1->y + pObj1->Height
	};

	bResult = IsCollisionRectVsRect(&objRect0.min, &objRect0.max, &objRect1.min, &objRect1.max);

lb_return:
	return bResult;
}
BOOL ProcessCollisionAmmoVsEnemies(FLIGHT_OBJECT* pAmmo, ULONGLONG CurTick)
{
	BOOL	bCollision = FALSE;
	for (DWORD i = 0; i < g_dwCurEnemiesNum; i++)
	{
		FLIGHT_OBJECT*	pEnemy = g_ppEnemyList[i];
		if (IsCollisionFlightObjectVsFlightObject(pAmmo, pEnemy))
		{
			// 적 타격 성공
			OnHitEnemy(pEnemy, CurTick);

			bCollision = TRUE;
			break;
		}
	}
	return bCollision;
}
void ProcessEnemies()
{
	// 적이 맵 끝(바닥)에 도달하면 자동파괴
	DWORD	dwIndex = 0;
	while (dwIndex < g_dwCurEnemiesNum)
	{
		FLIGHT_OBJECT*	pEnemy = g_ppEnemyList[dwIndex];
		if (pEnemy->y >= g_iScreenHeight)
		{
			DeleteFlightObject(pEnemy);
			g_dwCurEnemiesNum--;
			g_ppEnemyList[dwIndex] = g_ppEnemyList[g_dwCurEnemiesNum];
			g_ppEnemyList[g_dwCurEnemiesNum] = nullptr;
		}
		else
		{
			dwIndex++;
		}
	}
	MoveEnemies();
	FillEnemies();
}
void OnHitEnemy(FLIGHT_OBJECT* pEnemy, ULONGLONG CurTick)
{
	// 지금 삭제하지 않고 상태만 바꾼다.
	ChangeFlightObjectStatusToDead(pEnemy, CurTick);
	AddScore(SCORE_PER_ONE_KILL);
}
DWORD AddScore(DWORD dwAddval)
{
	g_dwCurScore += dwAddval;
	return g_dwCurScore;
}
void MoveEnemies()
{
	static ULONGLONG PrvEnemyMoveTick = 0;
	ULONGLONG CurTick = GetTickCount64();
	if (CurTick - PrvEnemyMoveTick < ENEMY_MOVE_ACTION_DELAY_TICK)
	{
		return;
	}
	PrvEnemyMoveTick = CurTick;

	for (DWORD i = 0; i < g_dwCurEnemiesNum; i++)
	{
		MoveEmemy(g_ppEnemyList[i]);
	}
}

void FillEnemies()
{
	static ULONGLONG PrvFillEnemyTick = 0;

	if (g_dwCurEnemiesNum >= MAX_ENEMY_NUM)
	{
		return;
	}

	ULONGLONG CurTick = GetTickCount64();
	if (CurTick - PrvFillEnemyTick < 3000)
	{
		return;
	}
	PrvFillEnemyTick = CurTick;

	DWORD	dwNeyEnemyNum = MAX_ENEMY_NUM - g_dwCurEnemiesNum;
	for (DWORD i = 0; i < dwNeyEnemyNum; i++)
	{
		FLIGHT_OBJECT*	pEnemy = CreateEnemyRandom();
		g_ppEnemyList[g_dwCurEnemiesNum] = pEnemy;
		g_dwCurEnemiesNum++;
	}
}
void DeleteAllEnemies()
{
	for (DWORD i = 0; i < g_dwCurEnemiesNum; i++)
	{
		DeleteFlightObject(g_ppEnemyList[i]);
		g_ppEnemyList[i] = nullptr;
	}
	g_dwCurEnemiesNum = 0;
}
void DeleteAllAmmos()
{
	for (DWORD i = 0; i < g_dwCurAmmoNum; i++)
	{
		DeleteFlightObject(g_ppAmmoList[i]);
		g_ppAmmoList[i] = nullptr;
	}
	g_dwCurAmmoNum = 0;
}
void CleanupGame()
{
	DeleteAllEnemies();
	DeleteAllAmmos();
	if (g_pPlayer)
	{
		DeleteFlightObject(g_pPlayer);
		g_pPlayer = nullptr;
	}
	if (g_pBackBuffer)
	{
		delete[] g_pBackBuffer;
		g_pBackBuffer = nullptr;
	}
	if (g_pBackground)
	{
		delete[] g_pBackground;
		g_pBackground = nullptr;
	}
	if (g_pBackgroundLineBuffer)
	{
		delete[] g_pBackgroundLineBuffer;
		g_pBackgroundLineBuffer = nullptr;
	}
}
void OnKeyDown_Left()
{
	int a = 0;
}
void OnKeyUp_Left()
{
	int a = 0;
}
void OnKeyDown_Up()
{
	int a = 0;
}
void OnKeyUp_Up()
{
	int a = 0;
}

void OnKeyDown_Right()
{
	int a = 0;
}
void OnKeyUp_Right()
{
	int a = 0;
}

void OnKeyDown_Down()
{
	int a = 0;
}
void OnKeyUp_Down()
{
	int a = 0;
}
void OnKeyDown_Space()
{
	ShootFromPlayer();
}
void OnKeyUp_Space()
{
}
void ShootFromPlayer()
{
	if (g_dwCurAmmoNum >= MAX_AMMO_NUM)
		return;

	FLIGHT_OBJECT*	pAmmo = CreateAmmo(g_pPlayer);
	g_ppAmmoList[g_dwCurAmmoNum] = pAmmo;
	g_dwCurAmmoNum++;
}

void ClearScreen()
{
	COORD	pos = {};
	DWORD	dwWrittenChrs = 0;
	FillConsoleOutputCharacter(g_hOut, L'\0', (DWORD)(g_iScreenWidth * g_iScreenHeight), pos, &dwWrittenChrs);
}

void ClearBackBuffer()
{
	memcpy(g_pBackBuffer, g_pBackground, sizeof(WCHAR) * g_iScreenWidth * g_iScreenHeight);
	//for (DWORD i = 0; i < (DWORD)(g_iScreenWidth * g_iScreenHeight); i++)
	//{
	//	g_pBackBuffer[i] = L' ';
	//}
}
void Blt()
{
	static ULONGLONG	PrvTick = GetTickCount64();
	static DWORD	dwFrameCount = 0;

	dwFrameCount++;

	DWORD lenout = 0;
	COORD	pos = {};
	WriteConsoleOutputCharacter(g_hOut, g_pBackBuffer, g_iScreenWidth * g_iScreenHeight, pos, &lenout);

	ULONGLONG CurTick = GetTickCount64();
	if (CurTick - PrvTick > 1000)
	{
		UpdateFPS(dwFrameCount);
		dwFrameCount = 0;
		PrvTick = GetTickCount64();
	}
}
void UpdateFPS(DWORD dwFPS)
{
	g_dwCurFPS = dwFPS;
	//WCHAR	wchTxt[32];
	//swprintf_s(wchTxt, L"FPS:%u\n", dwFPS);
	//OutputDebugStringW(wchTxt);
}
void DrawCursor()
{
	DWORD lenout;
	COORD	pos =
	{
		(SHORT)g_iCursorPosX,
		(SHORT)g_iCursorPosY
	};
	const WCHAR* wchTxt = L"x";
	WriteConsoleOutputCharacter(g_hOut, wchTxt, wcslen(wchTxt), pos, &lenout);

}

void DrawFPS(int x, int y)
{
	DWORD	dwOffset = x + y * g_iScreenWidth;
	WCHAR*	wchDest = g_pBackBuffer + dwOffset;
	DWORD	dwDesBufferLen = (DWORD)((g_iScreenWidth * g_iScreenHeight - (int)dwOffset));

	WCHAR	wchTxt[32];
	DWORD	dwLen = swprintf_s(wchTxt, L"FPS:%u", g_dwCurFPS);
	memcpy(wchDest, wchTxt, sizeof(WCHAR) * dwLen);
}
void DrawScore(int x, int y)
{
	DWORD	dwOffset = x + y * g_iScreenWidth;
	WCHAR*	wchDest = g_pBackBuffer + dwOffset;
	DWORD	dwDesBufferLen = (DWORD)((g_iScreenWidth * g_iScreenHeight - (int)dwOffset));

	WCHAR	wchTxt[32];
	DWORD	dwLen = swprintf_s(wchTxt, L"Score:%08u", g_dwCurScore);
	memcpy(wchDest, wchTxt, sizeof(WCHAR) * dwLen);
}