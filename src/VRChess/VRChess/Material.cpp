#include "stdafx.h"
#include "Material.h"
#include "Texture.h"
#include "DirectX11.h"
#include "COMHelpers.h"

Material::Material(Texture * t, uint32_t flags , D3D11_INPUT_ELEMENT_DESC * vertexDesc, int numVertexDesc,
	char* vertexShader, char* pixelShader, int vSize) 
	: Tex(t), VertexSize(vSize)
{
	D3D11_INPUT_ELEMENT_DESC defaultVertexDesc[] = {
		{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "Color",    0, DXGI_FORMAT_B8G8R8A8_UNORM,  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }, };

	// Use defaults if no shaders specified
	const char* defaultVertexShaderSrc =
		"float4x4 ProjView;  float4 MasterCol;"
		"void main(in  float4 Position  : POSITION,    in  float4 Color : COLOR0, in  float2 TexCoord  : TEXCOORD0,"
		"          out float4 oPosition : SV_Position, out float4 oColor: COLOR0, out float2 oTexCoord : TEXCOORD0)"
		"{   oPosition = mul(ProjView, Position); oTexCoord = TexCoord; "
		"    oColor = MasterCol * Color; }";
	const char* defaultPixelShaderSrc =
		"Texture2D Texture   : register(t0); SamplerState Linear : register(s0); "
		"float4 main(in float4 Position : SV_Position, in float4 Color: COLOR0, in float2 TexCoord : TEXCOORD0) : SV_Target"
		"{   float4 TexCol = Texture.Sample(Linear, TexCoord); "
		"    if (TexCol.a==0) clip(-1); " // If alpha = 0, don't draw
		"    return(Color * TexCol); }";

	// vertex shader for instanced stereo
	const char* instancedStereoVertexShaderSrc =
		"float4x4 modelViewProj[2];  float4 MasterCol;"
		"void main(in  float4 Position  : POSITION,    in  float4 Color : COLOR0, in  float2 TexCoord  : TEXCOORD0,"
		"          in  uint instanceID : SV_InstanceID, "
		"          out float4 oPosition : SV_Position, out float4 oColor: COLOR0, out float2 oTexCoord : TEXCOORD0,"
		"          out float oClipDist : SV_ClipDistance0, out float oCullDist : SV_CullDistance0)"
		"{"
		"   const float4 EyeClipPlane[2] = { { -1, 0, 0, 0 }, { 1, 0, 0, 0 } };"
		"   uint eyeIndex = instanceID & 1;"
		// transform to clip space for correct eye (includes offset and scale)
		"   oPosition = mul(modelViewProj[eyeIndex], Position); "
		// calculate distance from left/right clip plane (try setting to 0 to see why clipping is necessary)
		"   oCullDist = oClipDist = dot(EyeClipPlane[eyeIndex], oPosition);"
		"   oTexCoord = TexCoord; "
		"   oColor = MasterCol * Color;"
		"}";

	const char* vertexShaderString;
	const char* pixelShaderString;
	if (!vertexDesc)   vertexDesc = defaultVertexDesc;

	if (!vertexShader)
	{
		vertexShaderString = defaultVertexShaderSrc;
	}
	else
	{
		vertexShaderString = vertexShader;
	}

	if (!pixelShader)
	{
		pixelShaderString = defaultPixelShaderSrc;
	}
	else
	{
		pixelShaderString = pixelShader;
	}

	// Create vertex shader
	ID3DBlob * blobData;
	ID3DBlob * errorBlob = nullptr;
	HRESULT result = D3DCompile(vertexShaderString, strlen(vertexShaderString), 0, 0, 0, "main", "vs_4_0", 0, 0, &blobData, &errorBlob);
	if (FAILED(result))
	{
		MessageBoxA(nullptr, (char *)errorBlob->GetBufferPointer(), "Error compiling vertex shader", MB_OK);
		exit(-1);
	}
	DIRECTX.Device->CreateVertexShader(blobData->GetBufferPointer(), blobData->GetBufferSize(), nullptr, &VertexShader);

	// Create input layout
	DIRECTX.Device->CreateInputLayout(vertexDesc, numVertexDesc,
		blobData->GetBufferPointer(), blobData->GetBufferSize(), &InputLayout);
	blobData->Release();

	// Create vertex shader for instancing
	result = D3DCompile(instancedStereoVertexShaderSrc, strlen(instancedStereoVertexShaderSrc), 0, 0, 0, "main", "vs_4_0", 0, 0, &blobData, &errorBlob);
	if (FAILED(result))
	{
		MessageBoxA(nullptr, (char *)errorBlob->GetBufferPointer(), "Error compiling vertex shader", MB_OK);
		exit(-1);
	}
	DIRECTX.Device->CreateVertexShader(blobData->GetBufferPointer(), blobData->GetBufferSize(), nullptr, &VertexShaderInstanced);
	blobData->Release();

	// Create pixel shader
	D3DCompile(pixelShaderString, strlen(pixelShaderString), 0, 0, 0, "main", "ps_4_0", 0, 0, &blobData, 0);
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
}
Material::~Material()
{
	Release(VertexShader);
	Release(VertexShaderInstanced);
	Release(PixelShader);
	delete Tex; 
	Tex = nullptr;
	Release(InputLayout);
	Release(SamplerState);
	Release(Rasterizer);
	Release(DepthState);
	Release(BlendState);
}
