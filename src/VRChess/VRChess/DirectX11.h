#pragma once
#include "DepthBuffer.h"
#include "DataBuffer.h"

struct PixelShaderConstantBufferData
{
	XMFLOAT3 eyePos;
	XMFLOAT3 lightDir;
};

struct VertexShaderConstantBufferData
{
	XMMATRIX modelViewProj;
};

struct DirectX11
{
	HWND                     Window;
	bool                     Running;
	bool                     Key[256];
	int                      WinSizeW;
	int                      WinSizeH;
	ID3D11Device           * Device;
	ID3D11DeviceContext    * Context;
	IDXGISwapChain         * SwapChain;
	DepthBuffer            * MainDepthBuffer;
	ID3D11Texture2D        * BackBuffer;
	ID3D11RenderTargetView * BackBufferRT;
	// Fixed size buffer for shader constants, before copied into buffer
	static const int         UNIFORM_DATA_SIZE = 2000;
	unsigned char            UniformData[UNIFORM_DATA_SIZE];
	DataBuffer             * UniformBufferGen;
	DataBuffer				*PixelShaderConstantBuffer;
	HINSTANCE                hInstance;
public:
	DirectX11();
	~DirectX11();
	static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam);
	bool InitWindow(HINSTANCE hinst, LPCWSTR title);
	void CloseWindow();
	bool InitDevice(int vpW, int vpH, const LUID* pLuid, bool windowed = true, int scale = 1);
	void SetAndClearRenderTarget(ID3D11RenderTargetView * rendertarget, DepthBuffer * depthbuffer, float R = 0, float G = 0, float B = 0, float A = 1.0f);
	void SetAndClearRenderTarget(ID3D11RenderTargetView * rendertarget, ID3D11DepthStencilView* depthStencil, float R = 0, float G = 0, float B = 0, float A = 1.0f);
	void SetViewport(float vpX, float vpY, float vpW, float vpH);
	bool HandleMessages(void);
	void Run(bool(*MainLoop)(bool retryCreate));
	void ReleaseDevice();
};

extern DirectX11 DIRECTX;