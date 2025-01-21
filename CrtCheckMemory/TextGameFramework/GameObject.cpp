#include "stdafx.h"
#include "typedef.h"
#include "GameObject.h"
#include "Game.h"
#include "game_typedef.h"
#include "StaticData.h"



FLIGHT_OBJECT* CreatePlayer()
{
	FLIGHT_OBJECT*	pNewObject = new FLIGHT_OBJECT;
	pNewObject->pBits = g_FighterType0;
	pNewObject->x = (g_iScreenWidth / 2) - IMG_WIDTH_FIGHTER_TYPE0 / 2;
	pNewObject->y = (g_iScreenHeight / 2) - IMG_HEIGHT_FIGHTER_TYPE0 / 2;
	pNewObject->Width = IMG_WIDTH_FIGHTER_TYPE0;
	pNewObject->Height = IMG_HEIGHT_FIGHTER_TYPE0;
	pNewObject->status = FLIGHT_OBJECT_STATUS_ALIVE;

	return pNewObject;
}

FLIGHT_OBJECT* CreateEnemyRandom()
{
	FLIGHT_OBJECT*	pNewObject = new FLIGHT_OBJECT;
	pNewObject->pBits = g_pEnemyType0;
	
	int pos_x = rand() % (g_iScreenWidth - IMG_WIDTH_ENEMY_TYPE0);

	pNewObject->x = pos_x;
	pNewObject->y = 0;
	pNewObject->Width = IMG_WIDTH_ENEMY_TYPE0;
	pNewObject->Height = IMG_HEIGHT_ENEMY_TYPE0;
	pNewObject->status = FLIGHT_OBJECT_STATUS_ALIVE;

	return pNewObject;
}


FLIGHT_OBJECT* CreateAmmo(FLIGHT_OBJECT* pShooter)
{
	FLIGHT_OBJECT*	pAmmo = new FLIGHT_OBJECT;
	pAmmo->Width = 1;
	pAmmo->Height = 1;
	pAmmo->status = FLIGHT_OBJECT_STATUS_ALIVE;

	pAmmo->x = pShooter->x + pShooter->Width / 2;
	pAmmo->y = pShooter->y - 1;
	pAmmo->pBits = (WCHAR*)L"|";
	
	return pAmmo;
}
void DeleteFlightObject(FLIGHT_OBJECT* pObject)
{
	delete pObject;
}

void ChangeFlightObjectStatusToDead(FLIGHT_OBJECT* pObject, ULONGLONG CurTick)
{
	pObject->status = FLIGHT_OBJECT_STATUS_DEAD;
	pObject->Width = IMG_WIDTH_DESTROYED;
	pObject->Height = IMG_HEIGHT_DESTROYED;
	pObject->pBits = g_pDetroyed;
	pObject->DeadTick = CurTick;
}
BOOL MoveEmemy(FLIGHT_OBJECT* pObj)
{
	if (pObj->status != FLIGHT_OBJECT_STATUS_ALIVE)
		return FALSE;

	int move_x = (rand() % 3) - 2;
	pObj->x += move_x;

	if (pObj->x < 0)
	{
		pObj->x = 0;
	}
	if (pObj->x + pObj->Width > g_iScreenWidth)
	{
		pObj->x -= (pObj->x + pObj->Width - g_iScreenWidth);
	}
	pObj->y++;
	return TRUE;
}