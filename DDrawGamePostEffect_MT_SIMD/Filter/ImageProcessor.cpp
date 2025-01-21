#include "pch.h"
#include <process.h>
#include  "../Filter/CPU_Filter.h"
#include "../../Util/HybridCoreMapper.h"
#include "ImageProcessor.h"
#include <stdio.h>

UINT __stdcall ImageProcessingThread(void* pArg);


CImageProcessor::CImageProcessor()
{

}

BOOL CImageProcessor::Initialize()
{
	CHybridCoreMapper coreMapper;
	coreMapper.Initialize();

	
	DWORD	dwPhysicalCoreCount = 1;
	DWORD	dwLogicalCoreCount = 1;
	GetPhysicalCoreCount(&dwPhysicalCoreCount, &dwLogicalCoreCount);

	m_dwMaxThreadNum = dwLogicalCoreCount;
	//HYBRID_CORE_TYPE coreType = HYBRID_CORE_TYPE_PCORE;
	//HYBRID_CORE_TYPE coreType = HYBRID_CORE_TYPE_ECORE;
	HYBRID_CORE_TYPE coreType = HYBRID_CORE_TYPE_ALL;	// P코어/E코어 가리지 않고 다 사용한다.
	BOOL bHybridCpuSupported = coreMapper.IsHybridCpuSupported();
	if (bHybridCpuSupported)
	{
		ULONG CoreCount = coreMapper.GetCoreCount(coreType);
		if (CoreCount)
		{
			m_dwMaxThreadNum = CoreCount;
			m_bHybridCpuSupported = TRUE;
		}
		else
		{
			m_bHybridCpuSupported = FALSE;
		}
	}

	m_pThreadDescList = new THREAD_DESC[m_dwMaxThreadNum];
	memset(m_pThreadDescList, 0, sizeof(THREAD_DESC) * m_dwMaxThreadNum);

	for (DWORD i = 0; i < m_dwMaxThreadNum; i++)
	{
		m_pThreadDescList[i].pProcessor = this;
		m_pThreadDescList[i].dwThreadIndex = i;

		// for thread
		for (DWORD j = 0; j < THREAD_EVENT_TYPE_COUNT; j++)
		{
			m_pThreadDescList[i].hEventList[j] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		}
		UINT	uiThreadID = 0;
		m_pThreadDescList[i].hThread = (HANDLE)_beginthreadex(nullptr, 0, ImageProcessingThread, m_pThreadDescList + i, 0, &uiThreadID);

		WCHAR wchThreadName[64] = {};
		swprintf_s(wchThreadName, L"ImageProcessor[%u]", i);

		SetThreadDescription(m_pThreadDescList[i].hThread, wchThreadName);

		//SetThreadPriority(m_pThreadDescList[i].hThread, THREAD_PRIORITY_HIGHEST);
	}
	if (bHybridCpuSupported)
	{
		// 먼저 깨어나는 스레드가 P코어에 할당되도록 한다.
		DWORD dwThreadCount = 0;
		DWORD PCoreCount = coreMapper.GetCoreCount(HYBRID_CORE_TYPE_PCORE);
		for (DWORD i = 0; i < PCoreCount; i++)
		{
			coreMapper.SetThreadCoreType(m_pThreadDescList[dwThreadCount].hThread, HYBRID_CORE_TYPE_PCORE);
			dwThreadCount++;
		}
		// 늦게 깨어나는 스레드가 E코어에 할당되도록 한다.
		DWORD ECoreCount = coreMapper.GetCoreCount(HYBRID_CORE_TYPE_ECORE);
		for (DWORD i = 0; i < ECoreCount; i++)
		{
			coreMapper.SetThreadCoreType(m_pThreadDescList[dwThreadCount].hThread, HYBRID_CORE_TYPE_ECORE);
			dwThreadCount++;
		}
	}
	m_hCompletedEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	
#ifdef MULTI_THREAD_CONSIDER_HYBRID_CORE
	m_dwMaxWorkItemNum = m_dwMaxThreadNum * m_dwActiveThreadCountMulConst;	// P 코어와 E코어의 성능차를 감안하여 스레드개수x4개 정도로 처리할 작업개수를 나눈다.
	m_pWorkItemList = new THREAD_WORK_ITEM[m_dwMaxWorkItemNum];
	memset(m_pWorkItemList, 0, sizeof(THREAD_WORK_ITEM) * m_dwMaxWorkItemNum);
#endif
	m_dwActiveThreadNum = m_dwMaxThreadNum;


	return TRUE;
}
#ifdef MULTI_THREAD_CONSIDER_HYBRID_CORE
void CImageProcessor::ProcessByThread(DWORD dwThreadIndex)
{
	__m128 bw_mask_xmmword = { 0.11f, 0.59f, 0.3f, 0.0f };
	__m128i max_xmmword_i = _mm_cvtepu8_epi32(_mm_insert_epi32(_mm_setzero_si128(), -1, 0));	// 255 | 255 | 255 | 255
	while (1)
	{
		LONG lCurCount = _InterlockedIncrement(&m_lProcessingtemCount);
		lCurCount--;
		if (lCurCount >= m_dwPendingProcessingItemCount)
			break;

		THREAD_WORK_ITEM* pWorkItem = m_pWorkItemList + lCurCount;
		if (m_bUseSIMD)
		{
			CPU_Edge_Filter_SSE_MT(m_pImageDest, m_dwImageDestPitch, m_pImageSrc, m_dwImageWidth, m_dwImageHeight, m_dwSrcPitch, pWorkItem->dwStartY, pWorkItem->dwHeight, bw_mask_xmmword, max_xmmword_i);
		}
		else
		{
			CPU_Edge_Filter_MT(m_pImageDest, m_dwImageDestPitch, m_pImageSrc, m_dwImageWidth, m_dwImageHeight, m_dwSrcPitch, pWorkItem->dwStartY, pWorkItem->dwHeight);
		}

	}
	LONG lCurCount = _InterlockedDecrement(&m_lProcessingThreadCount);
	if (!lCurCount)
	{
		SetEvent(m_hCompletedEvent);
	}
}
#else
void CImageProcessor::ProcessByThread(DWORD dwStartYPerThread, DWORD dwHeightPerThread)
{
	__m128 bw_mask_xmmword = { 0.11f, 0.59f, 0.3f, 0.0f };
	__m128i max_xmmword_i = _mm_cvtepu8_epi32(_mm_insert_epi32(_mm_setzero_si128(), -1, 0));	// 255 | 255 | 255 | 255
	if (m_bUseSIMD)
	{
		CPU_Edge_Filter_SSE_MT(m_pImageDest, m_dwImageDestPitch, m_pImageSrc, m_dwImageWidth, m_dwImageHeight, m_dwSrcPitch, dwStartYPerThread, dwHeightPerThread, bw_mask_xmmword, max_xmmword_i);
	}
	else
	{
		CPU_Edge_Filter_MT(m_pImageDest, m_dwImageDestPitch, m_pImageSrc, m_dwImageWidth, m_dwImageHeight, m_dwSrcPitch, dwStartYPerThread, dwHeightPerThread);
	}
	LONG lCurCount = _InterlockedDecrement(&m_lProcessingThreadCount);
	if (!lCurCount)
	{
		SetEvent(m_hCompletedEvent);
	}
}
#endif


void CImageProcessor::ProcessImage(char* pDest, DWORD dwDestPitch, const char* pSrc, DWORD dwImageWidth, DWORD dwImageHeight, DWORD dwSrcPitch)
{
	if (m_bUseMT)
	{
		m_pImageDest = pDest;
		m_dwImageDestPitch = dwDestPitch;
		m_pImageSrc = pSrc;
		m_dwImageWidth = dwImageWidth;
		m_dwImageHeight = dwImageHeight;
		m_dwSrcPitch = dwSrcPitch;

	#ifdef MULTI_THREAD_CONSIDER_HYBRID_CORE
		DWORD dwWorkItemCount = m_dwActiveThreadNum * m_dwActiveThreadCountMulConst;	// P 코어와 E코어의 성능차를 감안하여 스레드개수x4개 정도로 처리할 작업개수를 나눈다.
		if (dwWorkItemCount > m_dwMaxWorkItemNum)
		{
			dwWorkItemCount = m_dwMaxWorkItemNum;
		}
		DWORD	dwHeightPerThread = dwImageHeight / dwWorkItemCount;
		DWORD	dwReservedHeight = dwImageHeight % dwWorkItemCount;

		DWORD dwStartY = 0;
		m_dwPendingProcessingItemCount = dwWorkItemCount;
		m_lProcessingtemCount = 0;

		for (DWORD i = 0; i < dwWorkItemCount; i++)
		{
			m_pWorkItemList[i].dwStartY = dwStartY;
			m_pWorkItemList[i].dwHeight = dwHeightPerThread;
			dwStartY += dwHeightPerThread;
		}
		m_pWorkItemList[0].dwHeight += dwReservedHeight;
		m_lProcessingThreadCount = m_dwActiveThreadNum;
		for (DWORD i = 0; i < m_dwActiveThreadNum; i++)
		{
			SetEvent(m_pThreadDescList[i].hEventList[THREAD_EVENT_TYPE_PROCESS]);
		}
	#else
		DWORD	dwHeightPerThread = dwImageHeight / m_dwActiveThreadNum;
		DWORD	dwReservedHeight = dwImageHeight % m_dwActiveThreadNum;

		DWORD dwStartY = 0;
		m_lProcessingThreadCount = m_dwActiveThreadNum;
		for (DWORD i = 0; i < m_dwActiveThreadNum; i++)
		{
			m_pThreadDescList[i].dwStartY = dwStartY;
			m_pThreadDescList[i].dwHeight = dwHeightPerThread;
			dwStartY += dwHeightPerThread;

			if (i == m_dwActiveThreadNum - 1)
			{
				// 이미지의 세로줄 수가 처리해야할 스레드 개수로 나눠떨어지지 않을 경우 남는 라인 수는 마지막 스레드가 처리
				m_pThreadDescList[i].dwHeight += dwReservedHeight;
			}
			SetEvent(m_pThreadDescList[i].hEventList[THREAD_EVENT_TYPE_PROCESS]);
		}
	#endif
		WaitForSingleObject(m_hCompletedEvent, INFINITE);
	}
	else
	{
		if (m_bUseSIMD)
		{
			__m128 bw_mask_xmmword = { 0.11f, 0.59f, 0.3f, 0.0f };
			__m128i max_xmmword_i = _mm_cvtepu8_epi32(_mm_insert_epi32(_mm_setzero_si128(), -1, 0));	// 255 | 255 | 255 | 255
			CPU_Edge_Filter_SSE_MT(pDest, dwDestPitch, pSrc, dwImageWidth, dwImageHeight, dwSrcPitch, 0, dwImageHeight, bw_mask_xmmword, max_xmmword_i);
		}
		else
		{
			CPU_Edge_Filter_MT(pDest, dwDestPitch, pSrc, dwImageWidth, dwImageHeight, dwSrcPitch, 0, dwImageHeight);
		}
	}
}
void CImageProcessor::Cleanup()
{
	if (m_pThreadDescList)
	{
		for (DWORD i = 0; i < m_dwMaxThreadNum; i++)
		{
			SetEvent(m_pThreadDescList[i].hEventList[THREAD_EVENT_TYPE_DESTROY]);
			WaitForSingleObject(m_pThreadDescList[i].hThread, INFINITE);
			CloseHandle(m_pThreadDescList[i].hThread);

			for (DWORD j = 0; j < THREAD_EVENT_TYPE_COUNT; j++)
			{
				CloseHandle(m_pThreadDescList[i].hEventList[j]);
				m_pThreadDescList[i].hEventList[j] = nullptr;
			}
		}

		delete[] m_pThreadDescList;
		m_pThreadDescList = nullptr;
	}
	if (m_hCompletedEvent)
	{
		CloseHandle(m_hCompletedEvent);
		m_hCompletedEvent = nullptr;
	}
#ifdef MULTI_THREAD_CONSIDER_HYBRID_CORE
	if (m_pWorkItemList)
	{
		delete[] m_pWorkItemList;
		m_pWorkItemList = nullptr;
	}
#endif
}
CImageProcessor::~CImageProcessor()
{
	Cleanup();
}

UINT __stdcall ImageProcessingThread(void* pArg)
{
	THREAD_DESC* pThreadDesc = (THREAD_DESC*)pArg;
	CImageProcessor* pImageProcessor = pThreadDesc->pProcessor;
	DWORD dwThreadIndex = pThreadDesc->dwThreadIndex;

	while (1)
	{
		DWORD dwEventIndex = WaitForMultipleObjects(THREAD_EVENT_TYPE_COUNT, pThreadDesc->hEventList, FALSE, INFINITE);
		switch (dwEventIndex)
		{
			case THREAD_EVENT_TYPE_PROCESS:
			#ifdef MULTI_THREAD_CONSIDER_HYBRID_CORE
				pImageProcessor->ProcessByThread(dwThreadIndex);
			#else
				pImageProcessor->ProcessByThread(pThreadDesc->dwStartY, pThreadDesc->dwHeight);
			#endif
				break;
			case THREAD_EVENT_TYPE_DESTROY:
				goto lb_exit;
		}
	}
lb_exit:
	_endthreadex(0);
	return 0;
}