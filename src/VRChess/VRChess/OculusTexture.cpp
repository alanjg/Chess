#include "stdafx.h"
#include "OculusTexture.h"
#include "DirectX11.h"
#include "COMHelpers.h"

OculusTexture::OculusTexture() :
	m_session(nullptr),
	m_textureChain(nullptr),
	m_depthTextureChain(nullptr)
{
}

bool OculusTexture::Init(ovrSession session, int sizeW, int sizeH, int sampleCount, bool createDepth)
{
	m_session = session;

	// create color texture swap chain first
	{
		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = sizeW;
		desc.Height = sizeH;
		desc.MipLevels = 1;
		desc.SampleCount = sampleCount;
		desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.MiscFlags = ovrTextureMisc_DX_Typeless | ovrTextureMisc_AutoGenerateMips;
		desc.BindFlags = ovrTextureBind_DX_RenderTarget;
		desc.StaticImage = ovrFalse;

		ovrResult result = ovr_CreateTextureSwapChainDX(session, DIRECTX.Device, &desc, &m_textureChain);
		if (!OVR_SUCCESS(result))
			return false;

		int textureCount = 0;
		ovr_GetTextureSwapChainLength(m_session, m_textureChain, &textureCount);
		for (int i = 0; i < textureCount; ++i)
		{
			ID3D11Texture2D* tex = nullptr;
			ovr_GetTextureSwapChainBufferDX(m_session, m_textureChain, i, IID_PPV_ARGS(&tex));

			D3D11_RENDER_TARGET_VIEW_DESC rtvd = {};
			rtvd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			rtvd.ViewDimension = (sampleCount > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS
				: D3D11_RTV_DIMENSION_TEXTURE2D;
			ID3D11RenderTargetView* rtv;
			HRESULT hr = DIRECTX.Device->CreateRenderTargetView(tex, &rtvd, &rtv);
			VALIDATE((hr == ERROR_SUCCESS), "Error creating render target view");
			m_texRtv.push_back(rtv);
			tex->Release();
		}
	}

	// if requested, then create depth swap chain
	if (createDepth)
	{
		ovrTextureSwapChainDesc desc = {};
		desc.Type = ovrTexture_2D;
		desc.ArraySize = 1;
		desc.Width = sizeW;
		desc.Height = sizeH;
		desc.MipLevels = 1;
		desc.SampleCount = sampleCount;
		desc.Format = OVR_FORMAT_D32_FLOAT;
		desc.MiscFlags = ovrTextureMisc_None;
		desc.BindFlags = ovrTextureBind_DX_DepthStencil;
		desc.StaticImage = ovrFalse;

		ovrResult result = ovr_CreateTextureSwapChainDX(session, DIRECTX.Device, &desc, &m_depthTextureChain);
		if (!OVR_SUCCESS(result))
			return false;

		int textureCount = 0;
		ovr_GetTextureSwapChainLength(m_session, m_depthTextureChain, &textureCount);
		for (int i = 0; i < textureCount; ++i)
		{
			ID3D11Texture2D* tex = nullptr;
			ovr_GetTextureSwapChainBufferDX(m_session, m_depthTextureChain, i, IID_PPV_ARGS(&tex));

			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = (sampleCount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS
				: D3D11_DSV_DIMENSION_TEXTURE2D;
			dsvDesc.Texture2D.MipSlice = 0;

			ID3D11DepthStencilView* dsv;
			HRESULT hr = DIRECTX.Device->CreateDepthStencilView(tex, &dsvDesc, &dsv);
			VALIDATE((hr == ERROR_SUCCESS), "Error creating depth stencil view");
			m_texDsv.push_back(dsv);
			tex->Release();
		}
	}

	return true;
}

OculusTexture::~OculusTexture()
{
	for (int i = 0; i < (int)m_texRtv.size(); ++i)
	{
		Release(m_texRtv[i]);
	}
	for (int i = 0; i < (int)m_texDsv.size(); ++i)
	{
		Release(m_texDsv[i]);
	}
	if (m_textureChain)
	{
		ovr_DestroyTextureSwapChain(m_session, m_textureChain);
	}
	if (m_depthTextureChain)
	{
		ovr_DestroyTextureSwapChain(m_session, m_depthTextureChain);
	}
}

ID3D11RenderTargetView* OculusTexture::GetRTV()
{
	int index = 0;
	ovr_GetTextureSwapChainCurrentIndex(m_session, m_textureChain, &index);
	return m_texRtv[index];
}
ID3D11DepthStencilView* OculusTexture::GetDSV()
{
	int index = 0;
	ovr_GetTextureSwapChainCurrentIndex(m_session, m_depthTextureChain, &index);
	return m_texDsv[index];
}

// Commit changes
void OculusTexture::Commit()
{
	ovr_CommitTextureSwapChain(m_session, m_textureChain);
	ovr_CommitTextureSwapChain(m_session, m_depthTextureChain);
}


ovrTextureSwapChain OculusTexture::GetTextureChain()
{
	return m_textureChain;
}

ovrTextureSwapChain OculusTexture::GetDepthTextureChain()
{
	return m_depthTextureChain;
}