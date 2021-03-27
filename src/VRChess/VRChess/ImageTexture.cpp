#include "stdafx.h"
#include "ImageTexture.h"
#include "DirectX11.h"
#include <WICTextureLoader.h>

ImageTexture::ImageTexture()
{
}


ImageTexture::~ImageTexture()
{
}

bool ImageTexture::Init(const std::wstring& filename)
{
	std::wstring txf(filename.begin(), filename.end());
	HRESULT hr = CreateWICTextureFromFile(DIRECTX.Device, txf.c_str(), (ID3D11Resource**)&m_Tex, &m_TexSv);
	if (FAILED(hr))
	{
		MessageBox(NULL, L"no file", filename.c_str(), MB_OK);
		return false;
	}
	return true;
}