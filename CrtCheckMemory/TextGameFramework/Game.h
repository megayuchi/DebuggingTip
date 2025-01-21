#pragma once

BOOL InitializeGame(HANDLE hOut);
void GameLoop(BOOL* pbOutExit);
void CleanupGame();

extern int g_iScreenWidth;
extern int g_iScreenHeight;