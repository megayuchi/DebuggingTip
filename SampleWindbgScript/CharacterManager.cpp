#include "pch.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "../Util/LinkedList.h"
#include "CharacterManager.h"

CCharacterManager::CCharacterManager()
{
}

CHARACTER* CCharacterManager::CreateCharacter(DWORD dwID, const WCHAR* wchName, int iHP, int iMP)
{
	CHARACTER* pChr = new CHARACTER;
	memset(pChr, 0, sizeof(CHARACTER));

	pChr->dwID = dwID;
	wcscpy_s(pChr->wchName, wchName);
	pChr->iHP = iHP;
	pChr->iMP = iMP;
	pChr->dwSpaceIndex = (DWORD)(-1);

	pChr->Link.pItem = pChr;
	pChr->Link.pNext = nullptr;
	pChr->Link.pPrv = nullptr;

	LinkToLinkedListFIFO(&m_pCharacterLinkHead, &m_pCharacterLinkTail, &pChr->Link);
	m_dwChrCount++;
	return pChr;
}
void CCharacterManager::DeleteCharacter(CHARACTER* pChr)
{
	UnLinkFromLinkedList(&m_pCharacterLinkHead, &m_pCharacterLinkTail, &pChr->Link);
	delete pChr;
	m_dwChrCount--;
}
void CCharacterManager::DeleteAllCharacters()
{
	while (m_pCharacterLinkHead)
	{
		CHARACTER* pChr = (CHARACTER*)m_pCharacterLinkHead->pItem;
		DeleteCharacter(pChr);
	}
}

void CCharacterManager::Process()
{
	LINK_ITEM* pCur = m_pCharacterLinkHead;
	while (pCur)
	{
		CHARACTER* pChr = (CHARACTER*)pCur->pItem;
		if (pChr->dwSpaceIndex != (DWORD)(-1))
			__debugbreak();

		pCur = pCur->pNext;
	}
}
void CCharacterManager::OccurBug()
{
	DWORD dwMidIndex = (m_dwChrCount / 2);
	LINK_ITEM* pCur = m_pCharacterLinkHead;
	for (DWORD i = 0; i < dwMidIndex; i++)
	{
		pCur = pCur->pNext;
	}
	CHARACTER* pChr = (CHARACTER*)pCur->pItem;
	pChr->dwSpaceIndex = 0xff000000;
}
void CCharacterManager::Cleanup()
{
	DeleteAllCharacters();
}
CCharacterManager::~CCharacterManager()
{
	Cleanup();
}