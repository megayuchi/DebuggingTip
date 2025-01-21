#pragma once

#include <windows.h>
#include "Filter_typedef.h"
#include <intrin.h>
void CPU_BW_Filter(char* pDest, const char* pSrc, DWORD dwWidth, DWORD dwHeight);
void CPU_Edge_Filter(char* pDest, DWORD dwDestPitch, const char* pSrc, DWORD dwImageWidth, DWORD dwImageHeight, DWORD dwSrcPitch);

void CPU_Blur_Filter(char* pDest, const char* pSrc, DWORD dwWidth, DWORD dwHeight);
void CPU_Edge_Filter_MT(char* pDest, DWORD dwDestPitch, const char* pSrc, DWORD dwImageWidth, DWORD dwImageHeight, DWORD dwSrcPitch, DWORD dwStartYPerThread, DWORD dwHeightPerThread);
void CPU_Edge_Filter_SSE_MT(char* pDest, DWORD dwDestPitch, const char* pSrc, DWORD dwImageWidth, DWORD dwImageHeight, DWORD dwSrcPitch, DWORD dwStartYPerThread, DWORD dwHeightPerThread, __m128 bw_mask_xmmword, __m128i max_xmmword_i);

struct IMAGE_PROCESS_DESC
{
	char*		pSrc;
	char*		pDest;
	FILTER_TYPE	FilterType;
	DWORD		dwThreadNum;
	DWORD		Width;
	DWORD		Height;

};

inline BOOL Is4BytesAligned(DWORD_PTR addr)
{
	BOOL	bResult = FALSE;
	bResult = (addr & 3) == 0;
	return bResult;
}

inline BOOL Is8BytesAligned(DWORD_PTR addr)
{
	BOOL	bResult = FALSE;
	bResult = (addr & 7) == 0;
	return bResult;
}
inline BOOL Is16BytesAligned(DWORD_PTR addr)
{
	BOOL	bResult = FALSE;
	bResult = (addr & 15) == 0;
	return bResult;
}
inline BOOL Is32BytesAligned(DWORD_PTR addr)
{
	BOOL	bResult = FALSE;
	bResult = (addr & 31) == 0;
	return bResult;
}

//#define USE_AVX

