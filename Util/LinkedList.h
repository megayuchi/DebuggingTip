#pragma once

struct LINK_ITEM
{
	LINK_ITEM*		pPrv;
	LINK_ITEM*		pNext;
	void*			pItem;
};

void LinkToLinkedList(LINK_ITEM** ppHead, LINK_ITEM** ppTail, LINK_ITEM* pNew);
void LinkToLinkedListFIFO(LINK_ITEM** ppHead, LINK_ITEM** ppTail, LINK_ITEM* pNew);
void UnLinkFromLinkedList(LINK_ITEM** ppHead, LINK_ITEM** ppTail, LINK_ITEM* pDel);
