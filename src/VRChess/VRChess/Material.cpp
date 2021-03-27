#include "stdafx.h"
#include "Material.h"
#include "Texture.h"
#include "DirectX11.h"
#include "COMHelpers.h"

MaterialData::MaterialData()
{
	ambient.x = ambient.y = ambient.z = 1.0;
	ambient.w = 1.0;
	diffuse.x = diffuse.y = diffuse.z = diffuse.w = 1.0;
	specular.x = specular.y = specular.z = specular.w = 1.0;
	specPower = 0;
}

Material::Material(Texture * t, MaterialData* materialData)
	: Tex(t)
{
	uint32_t flags = MAT_WRAP | MAT_TRANS;
	int numVertexDesc = 4;
	
	VertexSize = 36; 

	D3D11_INPUT_ELEMENT_DESC vertexDesc[] = {
		{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "Color",    0, DXGI_FORMAT_B8G8R8A8_UNORM,  0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
	};

	// Create vertex shader
	ID3DBlob * blobData;
	ID3DBlob * errorBlob = nullptr;

	HRESULT result = D3DCompileFromFile(L"PieceVertexShader.hlsl", NULL, NULL, "main", "vs_5_0", 0, 0, &blobData, &errorBlob);
	if (FAILED(result))
	{
		const char* errorMessage = errorBlob != nullptr ? (char*)errorBlob->GetBufferPointer() : "No error message available";
		MessageBoxA(nullptr, errorMessage, "Error compiling vertex shader", MB_OK);
		exit(-1);
	}

	DIRECTX.Device->CreateVertexShader(blobData->GetBufferPointer(), blobData->GetBufferSize(), nullptr, &VertexShader);

	// Create input layout
	DIRECTX.Device->CreateInputLayout(vertexDesc, numVertexDesc,
		blobData->GetBufferPointer(), blobData->GetBufferSize(), &InputLayout);
	blobData->Release();

	// Create pixel shader
	result = D3DCompileFromFile(L"PiecePixelShader.hlsl", NULL, NULL, "main", "ps_5_0", 0, 0, &blobData, &errorBlob);
	if (FAILED(result))
	{
		MessageBoxA(nullptr, (char *)errorBlob->GetBufferPointer(), "Error compiling pixel shader", MB_OK);
		exit(-1);
	}

	DIRECTX.Device->CreatePixelShader(blobData->GetBufferPointer(), blobData->GetBufferSize(), nullptr, &PixelShader);
	blobData->Release();

	// Create sampler state
	D3D11_SAMPLER_DESC ss; memset(&ss, 0, sizeof(ss));
	ss.AddressU = ss.AddressV = ss.AddressW = flags & MAT_WRAP ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_BORDER;
	ss.Filter = D3D11_FILTER_ANISOTROPIC;
	ss.MaxAnisotropy = 8;
	ss.MaxLOD = 15;
	DIRECTX.Device->CreateSamplerState(&ss, &SamplerState);

	// Create rasterizer
	D3D11_RASTERIZER_DESC rs; memset(&rs, 0, sizeof(rs));
	rs.AntialiasedLineEnable = rs.DepthClipEnable = true;
	rs.CullMode = flags & MAT_NOCULL ? D3D11_CULL_NONE : D3D11_CULL_BACK;
	rs.FillMode = flags & MAT_WIRE ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	DIRECTX.Device->CreateRasterizerState(&rs, &Rasterizer);

	// Create depth state
	D3D11_DEPTH_STENCIL_DESC dss;
	memset(&dss, 0, sizeof(dss));
	dss.DepthEnable = true;
	dss.DepthFunc = flags & MAT_ZALWAYS ? D3D11_COMPARISON_ALWAYS : D3D11_COMPARISON_LESS;
	dss.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DIRECTX.Device->CreateDepthStencilState(&dss, &DepthState);

	//Create blend state - trans or otherwise
	D3D11_BLEND_DESC bm;
	memset(&bm, 0, sizeof(bm));
	bm.RenderTarget[0].BlendEnable = flags & MAT_TRANS ? true : false;
	bm.RenderTarget[0].BlendOp = bm.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bm.RenderTarget[0].SrcBlend = bm.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	bm.RenderTarget[0].DestBlend = bm.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	bm.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	DIRECTX.Device->CreateBlendState(&bm, &BlendState);

	m_materialData = (MaterialData*)_aligned_malloc(sizeof(MaterialData), 16);
	if (materialData != nullptr)
	{
		memcpy(m_materialData, materialData, sizeof(MaterialData));
	}
	else
	{
		MaterialData blank;
		memcpy(m_materialData, &blank, sizeof(MaterialData));
	}
	materialBuffer.reset(new DataBuffer(DIRECTX.Device, D3D11_BIND_CONSTANT_BUFFER, m_materialData, sizeof(MaterialData)));
}

Material::~Material()
{
	Release(VertexShader);
	Release(PixelShader);
	delete Tex; 
	Tex = nullptr;
	Release(InputLayout);
	Release(SamplerState);
	Release(Rasterizer);
	Release(DepthState);
	Release(BlendState);
	_aligned_free(m_materialData);
}
