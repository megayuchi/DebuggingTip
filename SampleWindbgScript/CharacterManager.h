#pragma once


struct CHARACTER
{
	DWORD dwID;
	int iHP;
	int iMP;
	WCHAR wchName[32];
	DWORD dwSpaceIndex;
	LINK_ITEM Link;
};
class CCharacterManager
{
	LINK_ITEM* m_pCharacterLinkHead = nullptr;
	LINK_ITEM* m_pCharacterLinkTail = nullptr;
	DWORD m_dwChrCount = 0;

	void	DeleteCharacter(CHARACTER* pChr);
	void	Cleanup();
public:
	CHARACTER*	CreateCharacter(DWORD dwID, const WCHAR* wchName, int iHP, int iMP);
	void	DeleteAllCharacters();
	void	Process();
	void	OccurBug();
	CCharacterManager();
	~CCharacterManager();


};

