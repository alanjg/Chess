#pragma once
class Texture
{
protected:
	ID3D11Texture2D* m_Tex;
	ID3D11ShaderResourceView* m_TexSv;
	ID3D11RenderTargetView* m_TexRtv;
	int m_SizeW, m_SizeH, m_MipLevels;
public:

	Texture();
	void Init(int sizeW, int sizeH, bool rendertarget, int mipLevels, int sampleCount);
	Texture(int sizeW, int sizeH, bool rendertarget, int mipLevels = 1, int sampleCount = 1);
	

	virtual ~Texture();
	void FillTexture(uint32_t * pix);
	static void ConvertToSRGB(uint32_t * linear);

	ID3D11Texture2D* GetTexture();
	ID3D11ShaderResourceView* GetShaderResourceView();
	int GetWidth();
	int GetHeight();
};

