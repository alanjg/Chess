#include "stdafx.h"
#include "GeneratedTexture.h"
#include "DirectX11.h"
#include "COMHelpers.h"

GeneratedTexture::GeneratedTexture(bool rendertarget, int sizeW, int sizeH, int autoFillData, int sampleCount)
{
	Init(sizeW, sizeH, rendertarget, autoFillData ? 8 : 1, sampleCount);
	if (autoFillData) AutoFillTexture(autoFillData);
}


void GeneratedTexture::AutoFillTexture(int autoFillData)
{
	uint32_t * pix = (uint32_t *)malloc(sizeof(uint32_t) *  GetWidth() * GetHeight());
	for (int j = 0; j < GetHeight(); j++)
		for (int i = 0; i < GetWidth(); i++)
		{
			uint32_t * curr = &pix[j*GetWidth() + i];
			switch (autoFillData)
			{
			case(AUTO_WALL): *curr = (((j / 4 & 15) == 0) || (((i / 4 & 15) == 0) && ((((i / 4 & 31) == 0) ^ ((j / 4 >> 4) & 1)) == 0))) ?
				0xff3c3c3c : 0xffb4b4b4; break;
			case(AUTO_FLOOR): *curr = (((i >> 7) ^ (j >> 7)) & 1) ? 0xffb4b4b4 : 0xff505050;   break;
			case(AUTO_CEILING): *curr = (i / 4 == 0 || j / 4 == 0) ? 0xff505050 : 0xffb4b4b4;  break;
			case(AUTO_WHITE): *curr = 0xffffffff;                                              break;
			case(AUTO_GRADE_256): *curr = 0xff000000 + i * 0x010101;                             break;
			case(AUTO_GRID): *curr = (i<4) || (i>(GetWidth() - 5)) || (j<4) || (j>(GetHeight() - 5)) ? 0xffffffff : 0xff000000; break;
			default: *curr = 0xffffffff;              break;
			}
		}

	FillTexture(pix);
	free(pix);
}