#include "pch.h"
#include "LinkedList.h"



void LinkToLinkedList(LINK_ITEM** ppHead, LINK_ITEM** ppTail, LINK_ITEM* pNew)
{
	if (!*ppHead)
	{
		*ppTail = *ppHead = pNew;
		(*ppHead)->pNext = nullptr;
		(*ppHead)->pPrv = nullptr;

	}
	else
	{
	//#ifdef _DEBUG
		if (*ppHead == pNew)
			__debugbreak();
	//#endif
		pNew->pNext = (*ppHead);
		(*ppHead)->pPrv = pNew;
		(*ppHead) = pNew;
		pNew->pPrv = nullptr;
	}
}
void LinkToLinkedListFIFO(LINK_ITEM** ppHead, LINK_ITEM** ppTail, LINK_ITEM* pNew)
{
	if (!*ppHead)
	{
		*ppTail = *ppHead = pNew;
		(*ppHead)->pNext = nullptr;
		(*ppHead)->pPrv = nullptr;

	}
	else
	{
		pNew->pPrv = (*ppTail);
		(*ppTail)->pNext = pNew;
		(*ppTail) = pNew;
		pNew->pNext = nullptr;


	}
}

void UnLinkFromLinkedList(LINK_ITEM** ppHead, LINK_ITEM** ppTail, LINK_ITEM* pDel)
{
	LINK_ITEM*	pPrv = pDel->pPrv;
	LINK_ITEM*	pNext = pDel->pNext;

#ifdef	_DEBUG
	if (pDel->pPrv)
	{
		if (pDel->pPrv->pNext != pDel)
			__debugbreak();
	}
#endif

	if (pDel->pPrv)
		pDel->pPrv->pNext = pDel->pNext;
	else
	{
#ifdef _DEBUG
		if (pDel != (*ppHead))
			__debugbreak();
#endif
		(*ppHead) = pNext;
	}

	if (pDel->pNext)
		pDel->pNext->pPrv = pDel->pPrv;
	else
	{
#ifdef _DEBUG
		if (pDel != (*ppTail))
			__debugbreak();
#endif
		(*ppTail) = pPrv;
	}
	pDel->pPrv = nullptr;
	pDel->pNext = nullptr;

}