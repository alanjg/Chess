#pragma once
#include "Texture.h"
class GeneratedTexture : public Texture
{
public:
	enum 
	{ 
		AUTO_WHITE = 1, 
		AUTO_WALL, 
		AUTO_FLOOR, 
		AUTO_CEILING, 
		AUTO_GRID, 
		AUTO_GRADE_256 
	};

	GeneratedTexture(bool rendertarget, int sizeW, int sizeH, int autoFillData = 0, int sampleCount = 1);
	void AutoFillTexture(int autoFillData);
};

