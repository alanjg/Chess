#include "stdafx.h"
#include "Texture.h"
#include "DirectX11.h"
#include "COMHelpers.h"

Texture::Texture() : m_Tex(nullptr), m_TexSv(nullptr), m_TexRtv(nullptr)
{
}

void Texture::Init(int sizeW, int sizeH, bool rendertarget, int mipLevels, int sampleCount)
{
	m_SizeW = sizeW;
	m_SizeH = sizeH;
	m_MipLevels = mipLevels;

	D3D11_TEXTURE2D_DESC dsDesc;
	dsDesc.Width = m_SizeW;
	dsDesc.Height = m_SizeH;
	dsDesc.MipLevels = m_MipLevels;
	dsDesc.ArraySize = 1;
	dsDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dsDesc.SampleDesc.Count = sampleCount;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;
	dsDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (rendertarget) dsDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;

	DIRECTX.Device->CreateTexture2D(&dsDesc, nullptr, &m_Tex);
	DIRECTX.Device->CreateShaderResourceView(m_Tex, nullptr, &m_TexSv);
	m_TexRtv = nullptr;
	if (rendertarget) DIRECTX.Device->CreateRenderTargetView(m_Tex, nullptr, &m_TexRtv);
}

Texture::Texture(int sizeW, int sizeH, bool rendertarget, int mipLevels, int sampleCount)
{
	Init(sizeW, sizeH, rendertarget, mipLevels, sampleCount);
}

Texture::~Texture()
{
	Release(m_TexRtv);
	Release(m_TexSv);
	Release(m_Tex);
}

void Texture::FillTexture(uint32_t * pix)
{
	//Make local ones, because will be reducing them
	int sizeW = m_SizeW;
	int sizeH = m_SizeH;
	for (int level = 0; level < m_MipLevels; level++)
	{
		DIRECTX.Context->UpdateSubresource(m_Tex, level, nullptr, (unsigned char *)pix, sizeW * 4, sizeH * sizeW * 4);

		for (int j = 0; j < (sizeH & ~1); j += 2)
		{
			uint8_t* psrc = (uint8_t *)pix + (sizeW * j * 4);
			uint8_t* pdest = (uint8_t *)pix + (sizeW * j);
			for (int i = 0; i < sizeW >> 1; i++, psrc += 8, pdest += 4)
			{
				pdest[0] = (((int)psrc[0]) + psrc[4] + psrc[sizeW * 4 + 0] + psrc[sizeW * 4 + 4]) >> 2;
				pdest[1] = (((int)psrc[1]) + psrc[5] + psrc[sizeW * 4 + 1] + psrc[sizeW * 4 + 5]) >> 2;
				pdest[2] = (((int)psrc[2]) + psrc[6] + psrc[sizeW * 4 + 2] + psrc[sizeW * 4 + 6]) >> 2;
				pdest[3] = (((int)psrc[3]) + psrc[7] + psrc[sizeW * 4 + 3] + psrc[sizeW * 4 + 7]) >> 2;
			}
		}
		sizeW >>= 1;  sizeH >>= 1;
	}
}

void Texture::ConvertToSRGB(uint32_t * linear)
{
	uint32_t drgb[3];
	for (int k = 0; k < 3; k++)
	{
		float rgb = ((float)((*linear >> (k * 8)) & 0xff)) / 255.0f;
		rgb = pow(rgb, 2.2f);
		drgb[k] = (uint32_t)(rgb * 255.0f);
	}
	*linear = (*linear & 0xff000000) + (drgb[2] << 16) + (drgb[1] << 8) + (drgb[0] << 0);
}


ID3D11Texture2D* Texture::GetTexture()
{
	return m_Tex;
}

ID3D11ShaderResourceView* Texture::GetShaderResourceView()
{
	return m_TexSv;
}

int Texture::GetWidth()
{
	return m_SizeW;
}

int Texture::GetHeight()
{
	return m_SizeH;
}