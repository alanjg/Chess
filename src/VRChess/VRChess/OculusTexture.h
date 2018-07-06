#pragma once
class OculusTexture
{
	ovrSession               m_session;
	ovrTextureSwapChain      m_textureChain;
	ovrTextureSwapChain      m_depthTextureChain;
	std::vector<ID3D11RenderTargetView*> m_texRtv;
	std::vector<ID3D11DepthStencilView*> m_texDsv;

public:
	OculusTexture();
	~OculusTexture();
	bool Init(ovrSession session, int sizeW, int sizeH, int sampleCount, bool createDepth);
	ID3D11RenderTargetView* GetRTV();
	ID3D11DepthStencilView* GetDSV();

	ovrTextureSwapChain GetTextureChain();
	ovrTextureSwapChain GetDepthTextureChain();
	void Commit();
};

