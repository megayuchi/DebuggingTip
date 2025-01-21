#include "pch.h"
#include "CPU_Filter.h"
#include <intrin.h>

#pragma pack(push,1)
struct RGBA
{
	BYTE	b;
	BYTE	g;
	BYTE	r;
	BYTE	a;
};
#pragma pack(pop)

int SAMPLE_MASK_3_3_BARTLETT[3 * 3] =
{
	1,  1, 1,
	1,  1, 1,
	1,  1, 1
};

int SAMPLE_MASK_3_3_LAPLACIAN[3 * 3] =
{
	-1,-1,-1,
	-1, 8,-1,
	-1,-1,-1
};

int SAMPLE_MASK_3_3_GAUSSIAN[3 * 3] =
{
	1,  4, 1,
	4, 12, 4,
	1,  4, 1
};

const int MASK_3_3_WIDTH = 3;
const int MASK_3_3_HEIGHT = 3;
const int MASK_3_3_CENTER_X = 1;
const int MASK_3_3_CENTER_Y = 1;

int SAMPLE_MASK_5_5_GAUSSIAN[5 * 5] =
{
	1,  4, 8, 4, 1,
	4, 12,15,12, 4,
	8, 25,49,25, 8,
	4, 12,15,12, 4,
	1,  4, 8, 4, 1
};

const int MASK_5_5_WIDTH = 5;
const int MASK_5_5_HEIGHT = 5;
const int MASK_5_5_CENTER_X = 2;
const int MASK_5_5_CENTER_Y = 2;

__m128 g_bw_mask_xmmword = { 0.11f, 0.59f, 0.3f, 0.0f };

DWORD SampleEdgePixel32_CPU(const char* pBits, int iWidth, int iHeight, int iPitch, int sx, int sy, int* pMask, int iMaskWidth, int iMaskHeight, int iMaskCenterX, int iMaskCenterY);
DWORD __vectorcall SampleEdgePixel32_CPU_SSE(const char* pBits, int iWidth, int iHeight, int iPitch, int sx, int sy, int* pMask, int iMaskWidth, int iMaskHeight, int iMaskCenterX, int iMaskCenterY, __m128 bw_mask_xmmword, __m128i max_xmmword_i);

DWORD SampleBlurPixel32_CPU(const char* pBits, int iWidth, int iHeight, int iPitch, int sx, int sy, int* pMask, int iMaskWidth, int iMaskHeight, int iMaskCenterX, int iMaskCenterY)
{
	int		src_start_x = sx - iMaskCenterX;

	int		mask_start_x = 0;
	int		mask_start_y = 0;

	if (src_start_x < 0)
		mask_start_x = 0 - src_start_x;

	int		src_start_y = sy - iMaskCenterY;

	if (src_start_y < 0)
		mask_start_y = 0 - src_start_y;

	int		src_end_x = src_start_x + iMaskWidth;
	int		src_end_y = src_start_y + iMaskHeight;

	if (src_start_x < 0)
		src_start_x = 0;

	if (src_start_y < 0)
		src_start_y = 0;

	if (src_end_x > iWidth)
		src_end_x = iWidth;

	if (src_end_y > iHeight)
		src_end_y = iHeight;

	int		sample_width = src_end_x - src_start_x;
	int		sample_height = src_end_y - src_start_y;

	float	color[4];
	float	total_color[4] = { 0,0,0,0 };
	DWORD	R, G, B, A;

	float	weight;
	DWORD	dwPixelOut, dwPixelIn;

	float	total = 0.0f;
	for (int y = 0; y < sample_height; y++)
	{
		for (int x = 0; x < sample_width; x++)
		{
			total += pMask[(x + mask_start_x) + (y + mask_start_y) * iMaskWidth];
		}
	}


	DWORD*		pPixel;
	for (int y = 0; y < sample_height; y++)
	{
		for (int x = 0; x < sample_width; x++)
		{
			weight = pMask[(x + mask_start_x) + (y + mask_start_y) * iMaskWidth] / total;
			if (weight != 0.0f)
			{
				pPixel = (DWORD*)(pBits + ((x + src_start_x) << 2) + (y + src_start_y) * iPitch);


				dwPixelIn = *pPixel;

				color[3] = (float)((dwPixelIn & 0xff000000) >> 24);
				color[2] = (float)((dwPixelIn & 0x00ff0000) >> 16);
				color[1] = (float)((dwPixelIn & 0x0000ff00) >> 8);
				color[0] = (float)(dwPixelIn & 0x000000ff);

				for (int k = 0; k < 4; k++)
				{
					total_color[k] += (color[k] * weight);
				}
			}
		}
	}
	A = (DWORD)total_color[3];
	R = (DWORD)total_color[2];
	G = (DWORD)total_color[1];
	B = (DWORD)total_color[0];


	dwPixelOut = (A << 24) | (R << 16) | (G << 8) | B;


	return dwPixelOut;


}

DWORD SampleEdgePixel32_CPU(const char* pBits, int iWidth, int iHeight, int iPitch, int sx, int sy, int* pMask, int iMaskWidth, int iMaskHeight, int iMaskCenterX, int iMaskCenterY)
{
	int		src_start_x = sx - iMaskCenterX;

	int		mask_start_x = 0;
	int		mask_start_y = 0;

	if (src_start_x < 0)
		mask_start_x = 0 - src_start_x;

	int		src_start_y = sy - iMaskCenterY;

	if (src_start_y < 0)
		mask_start_y = 0 - src_start_y;

	int		src_end_x = src_start_x + iMaskWidth;
	int		src_end_y = src_start_y + iMaskHeight;

	if (src_start_x < 0)
		src_start_x = 0;

	if (src_start_y < 0)
		src_start_y = 0;

	if (src_end_x > iWidth)
		src_end_x = iWidth;

	if (src_end_y > iHeight)
		src_end_y = iHeight;

	int		sample_width = src_end_x - src_start_x;
	int		sample_height = src_end_y - src_start_y;

	int	color[4];
	int	total_color[4] = {};

	for (int y = 0; y < sample_height; y++)
	{
		for (int x = 0; x < sample_width; x++)
		{
			int weight = pMask[(x + mask_start_x) + (y + mask_start_y) * iMaskWidth];
			DWORD* pPixel = (DWORD*)(pBits + ((x + src_start_x) << 2) + (y + src_start_y) * iPitch);

			DWORD dwPixelIn = *pPixel;

			color[3] = (int)((dwPixelIn & 0xff000000) >> 24);
			color[2] = (int)((dwPixelIn & 0x00ff0000) >> 16);
			color[1] = (int)((dwPixelIn & 0x0000ff00) >> 8);
			color[0] = (int)(dwPixelIn & 0x000000ff);

			for (int k = 0; k < 4; k++)
			{
				total_color[k] += (color[k] * weight);
			}
		}
	}

	int	bw = (int)(((float)total_color[2] * 0.3f) + ((float)total_color[1] * 0.59f) + ((float)total_color[0] * 0.11f));
	bw = 255 - bw;

	if (bw > 255)
		bw = 255;

	if (bw < 0)
		bw = 0;


	return ((bw << 24) | (bw << 16) | (bw << 8) | bw);
}

DWORD __vectorcall SampleEdgePixel32_CPU_SSE(const char* pBits, int iWidth, int iHeight, int iPitch, int sx, int sy, int* pMask, int iMaskWidth, int iMaskHeight, int iMaskCenterX, int iMaskCenterY, __m128 bw_mask_xmmword, __m128i max_xmmword_i)
{
	int		src_start_x = sx - iMaskCenterX;

	int		mask_start_x = 0;
	int		mask_start_y = 0;

	if (src_start_x < 0)
		mask_start_x = 0 - src_start_x;

	int		src_start_y = sy - iMaskCenterY;

	if (src_start_y < 0)
		mask_start_y = 0 - src_start_y;

	int		src_end_x = src_start_x + iMaskWidth;
	int		src_end_y = src_start_y + iMaskHeight;

	if (src_start_x < 0)
		src_start_x = 0;

	if (src_start_y < 0)
		src_start_y = 0;

	if (src_end_x > iWidth)
		src_end_x = iWidth;

	if (src_end_y > iHeight)
		src_end_y = iHeight;

	int		sample_width = src_end_x - src_start_x;
	int		sample_height = src_end_y - src_start_y;

	__m128i sum_xmmword = _mm_setzero_si128();

	DWORD* 	pPixel = (DWORD*)(pBits + ((0 + src_start_x) << 2) + (0 + src_start_y) * iPitch);
	int*	pWeight = pMask + ((0 + mask_start_x) + (0 + mask_start_y) * iMaskWidth);
	for (int y = 0; y < sample_height; y++)
	{
		int x = 0;
	#ifdef USE_AVX
		if (x + 2 < sample_width)
		{
			if (Is8BytesAligned((DWORD_PTR)pPixel))
			{
			
				// 두 픽셀 처리
				//pWeight[0] = 1;
				//pWeight[1] = 2;

				//__m128i weight_xmmword_lo = _mm_set1_epi32(pWeight[0]);
				//__m128i weight_xmmword_hi = _mm_set1_epi32(pWeight[1]);
				__m128i weight_qword = _mm_loadl_epi64((__m128i*)pWeight); // 64비트 메모리에서 8바이트 로드
				__m128i weight_xmmword_lo = _mm_shuffle_epi32(weight_qword, 0b00000000);
				__m128i weight_xmmword_hi = _mm_shuffle_epi32(weight_qword, 0b01010101);
			
				__m256i weight_vmmword = _mm256_set_m128i(weight_xmmword_hi, weight_xmmword_lo);
			

				//pPixel[0] = (8<<24) | (4<<16) | (2<<8) | 1;
				//pPixel[1] = (128<<24) | (64<<16) | (32<<8) | 16;

				//__m128i src_dword_lo = _mm_loadu_si32(pPixel + 0);
				//__m128i src_dword_hi = _mm_loadu_si32(pPixel + 1);

				__m128i src_qword = _mm_loadl_epi64((__m128i*)pPixel);						// p1.argb(4bytes) | p0.argb(4bytes)
				__m128i src_xmmword_i_lo = _mm_cvtepu8_epi32(src_qword);					// p0.a | p0.b |p0.g | p0.r
				__m128i src_xmmword_i_hi = _mm_cvtepu8_epi32(_mm_srli_si128(src_qword, 4)); // p1.a | p1.b |p1.g | p1.r
				__m256i src_vmmword = _mm256_set_m128i(src_xmmword_i_hi, src_xmmword_i_lo);	// p1.a | p1.b |p1.g | p1.r | p0.a | p0.b |p0.g | p0.r

				//__m256i mul_vmmword = _mm256_mullo_epi32(src_vmmword, weight_vmmword);
				//for (DWORD i = 0; i < 8; i++)
				//{
				//	if (src_vmmword.m256i_i32[i] * weight_vmmword.m256i_i32[i] != mul_vmmword.m256i_i32[i])
				//		__debugbreak();
				//}
				__m256i mul_vmmword = _mm256_mullo_epi32(src_vmmword, weight_vmmword);
				__m128i mul_xmmword_lo = _mm256_castsi256_si128(mul_vmmword);		// 하위 128비트
				__m128i mul_xmmword_hi = _mm256_extracti128_si256(mul_vmmword, 1);	// 상위 128비트
			
				sum_xmmword = _mm_add_epi32(sum_xmmword, mul_xmmword_lo);
				sum_xmmword = _mm_add_epi32(sum_xmmword, mul_xmmword_hi);
				
				pPixel += 2;
				pWeight += 2;
				x += 2;
			}
			
		}
		while (x < sample_width)
		{	
			// 남은 1픽셀 처리
			//pPixel = (DWORD*)(pBits + ((x + src_start_x) << 2) + (y + src_start_y) * iPitch);
			//int* pWeight = pMask + ((x + mask_start_x) + (y + mask_start_y) * iMaskWidth);
			
			__m128i weight_xmmword = _mm_set1_epi32(*pWeight);
			__m128i src_dword = _mm_loadu_si32(pPixel);
			__m128i src_xmmword_i = _mm_cvtepu8_epi32(src_dword);

			//__m128i src_dword = _mm_loadu_epi32(pPixel);
			//__m128 src_xmmword = _mm_cvtepi32_ps(src_xmmword_i);
			
			__m128i mul_xmmword = _mm_mullo_epi32(src_xmmword_i, weight_xmmword);
			sum_xmmword = _mm_add_epi32(sum_xmmword, mul_xmmword);
			pPixel++;
			pWeight++;
			x++;
		}
	#else

		while (x < sample_width)
		{			
			__m128i weight_xmmword = _mm_set1_epi32(*pWeight);
			__m128i src_dword = _mm_loadu_si32(pPixel);
			__m128i src_xmmword_i = _mm_cvtepu8_epi32(src_dword);

			//__m128i src_dword = _mm_loadu_epi32(pPixel);
			//__m128 src_xmmword = _mm_cvtepi32_ps(src_xmmword_i);
			
			__m128i mul_xmmword = _mm_mullo_epi32(src_xmmword_i, weight_xmmword);
			sum_xmmword = _mm_add_epi32(sum_xmmword, mul_xmmword);
			pWeight++;
			pPixel++;
			x++;
		}
	#endif
		pPixel -= sample_width;
		pPixel = (DWORD*)((char*)pPixel + iPitch);
		pWeight -= sample_width;
		pWeight += iMaskWidth;
	}
	__m128i zero_xmmword_i = _mm_setzero_si128();	// 0 | 0 | 0 | 0

	
	//__m128 bw_xmmword = _mm_mul_ps(_mm_cvtepi32_ps(sum_xmmword), bw_mask_xmmword);
	//bw_xmmword = _mm_hadd_ps(bw_xmmword, bw_xmmword);
	//bw_xmmword = _mm_hadd_ps(bw_xmmword, bw_xmmword);
	__m128 bw_xmmword = _mm_dp_ps(_mm_cvtepi32_ps(sum_xmmword), bw_mask_xmmword, 0b01110111);

	__m128i bw_xmmword_i = _mm_cvttps_epi32(bw_xmmword);
	bw_xmmword_i = _mm_sub_epi32(max_xmmword_i, bw_xmmword_i);	// 흑백 반전

	bw_xmmword_i = _mm_min_epi32(bw_xmmword_i, max_xmmword_i);
	bw_xmmword_i = _mm_max_epi32(bw_xmmword_i, zero_xmmword_i);

	__m128i bw_xmmword_temp16 = _mm_packs_epi32(bw_xmmword_i, zero_xmmword_i);
	__m128i bw_result = _mm_packus_epi16(bw_xmmword_temp16, zero_xmmword_i);

	//__m128i bw_dword = _mm_cvtepi32_epi8(bw_xmmword_i);	avx512필요
	return (bw_result.m128i_u32[0] & 0x00ffffff);
}


void CPU_Blur_Filter(char* pDest, const char* pSrc, DWORD dwWidth, DWORD dwHeight)
{
	DWORD	dwPitch = dwWidth * 4;

	DWORD	start_y = 0;
	DWORD	end_y = start_y + dwHeight;

	for (DWORD y = start_y; y < end_y; y++)
	{
		for (DWORD x = 0; x < dwWidth; x++)
		{
			DWORD	dwPixel = SampleBlurPixel32_CPU(pSrc, dwWidth, dwHeight, dwPitch, x, y, SAMPLE_MASK_5_5_GAUSSIAN, MASK_5_5_WIDTH, MASK_5_5_HEIGHT, MASK_5_5_CENTER_X, MASK_5_5_CENTER_Y);

			DWORD*	pDestColor = (DWORD*)pDest + x + (y * dwWidth);
			*pDestColor = dwPixel;
		}
	}

}
void CPU_Edge_Filter(char* pDest, DWORD dwDestPitch, const char* pSrc, DWORD dwImageWidth, DWORD dwImageHeight, DWORD dwSrcPitch)
{
	DWORD	start_y = 0;
	DWORD	end_y = start_y + dwImageHeight;

	for (DWORD y = start_y; y < end_y; y++)
	{
		for (DWORD x = 0; x < dwImageWidth; x++)
		{
			DWORD	dwPixel = SampleEdgePixel32_CPU(pSrc, dwImageWidth, dwImageHeight, dwSrcPitch, x, y, SAMPLE_MASK_3_3_LAPLACIAN, MASK_3_3_WIDTH, MASK_3_3_HEIGHT, MASK_3_3_CENTER_X, MASK_3_3_CENTER_Y);
			DWORD*	pDestColor = (DWORD*)(pDest + (x*4) + (y * dwDestPitch));
			*pDestColor = dwPixel;
		}
	}
}

void CPU_Edge_Filter_MT(char* pDest, DWORD dwDestPitch, const char* pSrc, DWORD dwImageWidth, DWORD dwImageHeight, DWORD dwSrcPitch, DWORD dwStartYPerThread, DWORD dwHeightPerThread)
{
	DWORD	start_y = dwStartYPerThread;
	DWORD	end_y = start_y + dwHeightPerThread;

	for (DWORD y = start_y; y < end_y; y++)
	{
		for (DWORD x = 0; x < dwImageWidth; x++)
		{
			DWORD	dwPixel = SampleEdgePixel32_CPU(pSrc, dwImageWidth, dwImageHeight, dwSrcPitch, x, y, SAMPLE_MASK_3_3_LAPLACIAN, MASK_3_3_WIDTH, MASK_3_3_HEIGHT, MASK_3_3_CENTER_X, MASK_3_3_CENTER_Y);
			DWORD*	pDestColor = (DWORD*)(pDest + (x*4) + (y * dwDestPitch));
			*pDestColor = dwPixel;
		}
	}
}
void CPU_Edge_Filter_SSE_MT(char* pDest, DWORD dwDestPitch, const char* pSrc, DWORD dwImageWidth, DWORD dwImageHeight, DWORD dwSrcPitch, DWORD dwStartYPerThread, DWORD dwHeightPerThread, __m128 bw_mask_xmmword, __m128i max_xmmword_i)
{
	DWORD	start_y = dwStartYPerThread;
	DWORD	end_y = start_y + dwHeightPerThread;

	for (DWORD y = start_y; y < end_y; y++)
	{
		for (DWORD x = 0; x < dwImageWidth; x++)
		{
			DWORD	dwPixel = SampleEdgePixel32_CPU_SSE(pSrc, dwImageWidth, dwImageHeight, dwSrcPitch, x, y, SAMPLE_MASK_3_3_LAPLACIAN, MASK_3_3_WIDTH, MASK_3_3_HEIGHT, MASK_3_3_CENTER_X, MASK_3_3_CENTER_Y, bw_mask_xmmword, max_xmmword_i);
			DWORD*	pDestColor = (DWORD*)(pDest + (x*4) + (y * dwDestPitch));
			*pDestColor = dwPixel;
		}
	}

}
void CPU_BW_Filter(char* pDest, const char* pSrc, DWORD dwWidth, DWORD dwHeight)
{
	DWORD	dwPitch = dwWidth * 4;

	DWORD	start_y = 0;
	DWORD	end_y = start_y + dwHeight;

	for (DWORD y = start_y; y < end_y; y++)
	{
		for (DWORD x = 0; x < dwWidth; x++)
		{
			RGBA*	pSrcColor = (RGBA*)pSrc + x + (y * dwWidth);
			RGBA*	pDestColor = (RGBA*)pDest + x + (y * dwWidth);

			int		iBright = (int)(((float)pSrcColor->r * 0.3f) + ((float)pSrcColor->g * 0.59f) + ((float)pSrcColor->b * 0.11f));

			pDestColor->a = (BYTE)iBright;
			pDestColor->r = (BYTE)iBright;
			pDestColor->g = (BYTE)iBright;
			pDestColor->b = (BYTE)iBright;
		}
	}
}
