#pragma once
class DepthBuffer
{
	ID3D11DepthStencilView * m_TexDsv;
public:
	DepthBuffer(ID3D11Device * Device, int sizeW, int sizeH, int sampleCount = 1);
	~DepthBuffer();
	ID3D11DepthStencilView* GetDepthStencilView();
};

