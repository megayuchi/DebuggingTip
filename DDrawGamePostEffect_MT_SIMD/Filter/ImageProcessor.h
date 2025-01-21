#pragma once

#include "Filter_typedef.h"

#define MULTI_THREAD_CONSIDER_HYBRID_CORE

enum THREAD_EVENT_TYPE
{
	THREAD_EVENT_TYPE_PROCESS,
	THREAD_EVENT_TYPE_DESTROY,
	THREAD_EVENT_TYPE_COUNT
};
class CImageProcessor;
struct THREAD_DESC
{
	CImageProcessor* pProcessor;
	HANDLE hThread;
	DWORD dwThreadIndex;
	DWORD dwStartY;	// �����尡 ó���ؾ��� �̹����� ���� �� �ε���
	DWORD dwHeight;	// �����尡 ó���ؾ��� �̹����� ���� ��
	HANDLE hEventList[THREAD_EVENT_TYPE_COUNT];
	FILTER_TYPE filterType;
};
struct THREAD_WORK_ITEM
{
	DWORD dwStartY;	// �����尡 ó���ؾ��� �̹����� ���� �� �ε���
	DWORD dwHeight;	// �����尡 ó���ؾ��� �̹����� ���� ��
};
class CImageProcessor
{
	DWORD	m_dwMaxThreadNum = 0;
	DWORD	m_dwActiveThreadNum = 0;
	BOOL	m_bHybridCpuSupported = FALSE;
	THREAD_DESC*	m_pThreadDescList = nullptr;
	THREAD_WORK_ITEM*	m_pWorkItemList = nullptr;
	DWORD	m_dwMaxWorkItemNum = 0;

	HANDLE m_hCompletedEvent = nullptr;
	
	char* m_pImageDest = nullptr;
	DWORD m_dwImageDestPitch = 0;
	const char* m_pImageSrc = nullptr;
	DWORD m_dwImageWidth = 0;
	DWORD m_dwImageHeight = 0;
	DWORD m_dwSrcPitch = 0;
	BOOL m_bUseSIMD = FALSE;
	BOOL m_bUseMT = FALSE;
	
#ifdef MULTI_THREAD_CONSIDER_HYBRID_CORE
	DWORD m_dwActiveThreadCountMulConst = 4;	// P �ھ�� E�ھ��� �������� �����Ͽ� �����尳��x4�� ������ ó���� �۾������� ������.
	DWORD m_dwPendingProcessingItemCount = 0;
	LONG volatile m_lProcessingtemCount = 0;
#endif
	LONG volatile m_lProcessingThreadCount = 0;
	void	Cleanup();
public:
	BOOL	Initialize();
	void	ProcessImage(char* pDest, DWORD dwDestPitch, const char* pSrc, DWORD dwImageWidth, DWORD dwImageHeight, DWORD dwSrcPitch);
	void	SetActiveThreadCount(DWORD dwThreadCount) { m_dwActiveThreadNum = dwThreadCount; }
	DWORD	GetActiveThreadCount() const { return m_dwActiveThreadNum; }
	void	EnableMultThread(BOOL bSwitch) { m_bUseMT = bSwitch; }
	BOOL	IsMultiThreadEnabled() const { return m_bUseMT; }
	void	EnableSIMD(BOOL bSwitch) { m_bUseSIMD = bSwitch; }
	BOOL	IsSIMDEnabled() const { return m_bUseSIMD; }


#ifdef MULTI_THREAD_CONSIDER_HYBRID_CORE
	void	ProcessByThread(DWORD dwThreadIndex);
#else
	void	ProcessByThread(DWORD dwStartYPerThread, DWORD dwHeightPerThread);
#endif
	CImageProcessor();
	~CImageProcessor();
};



