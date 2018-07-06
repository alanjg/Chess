#pragma once
#include "GeneratedTexture.h"
struct Material
{
	ID3D11VertexShader      * VertexShader, *VertexShaderInstanced;
	ID3D11PixelShader       * PixelShader;
	Texture                 * Tex;
	ID3D11InputLayout       * InputLayout;
	UINT                      VertexSize;
	ID3D11SamplerState      * SamplerState;
	ID3D11RasterizerState   * Rasterizer;
	ID3D11DepthStencilState * DepthState;
	ID3D11BlendState        * BlendState;

	enum { MAT_WRAP = 1, MAT_WIRE = 2, MAT_ZALWAYS = 4, MAT_NOCULL = 8, MAT_TRANS = 16 };

public:
	Material(Texture * t, uint32_t flags = MAT_WRAP | MAT_TRANS, D3D11_INPUT_ELEMENT_DESC * vertexDesc = nullptr, int numVertexDesc = 3,
		char* vertexShader = nullptr, char* pixelShader = nullptr, int vSize = 24);

	~Material();
};