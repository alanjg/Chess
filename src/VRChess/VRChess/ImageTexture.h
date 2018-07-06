#pragma once
#include "Texture.h"
class ImageTexture : public Texture
{
	ID3D11ShaderResourceView* texture;
public:
	ImageTexture();
	virtual ~ImageTexture();
	bool Init(const std::wstring& filename);
};

