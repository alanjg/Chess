#pragma once
#include "GeneratedTexture.h"
#include "DataBuffer.h"

struct MaterialData
{
	XMFLOAT4A ambient;
	XMFLOAT4A diffuse;
	XMFLOAT4A specular;
	float specPower;
	MaterialData();
};

struct Material
{
	ID3D11VertexShader      * VertexShader;
	ID3D11PixelShader       * PixelShader;
	Texture                 * Tex;
	ID3D11InputLayout       * InputLayout;
	UINT                      VertexSize;
	ID3D11SamplerState      * SamplerState;
	ID3D11RasterizerState   * Rasterizer;
	ID3D11DepthStencilState * DepthState;
	ID3D11BlendState* BlendState;
	unique_ptr<DataBuffer>	materialBuffer;
	MaterialData* m_materialData;
	enum { MAT_WRAP = 1, MAT_WIRE = 2, MAT_ZALWAYS = 4, MAT_NOCULL = 8, MAT_TRANS = 16 };

public:
	Material(Texture * t, MaterialData* materialData = nullptr);
	~Material();
};