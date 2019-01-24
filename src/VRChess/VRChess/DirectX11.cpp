#include "stdafx.h"
#include "DirectX11.h"
#include "COMHelpers.h"

LRESULT CALLBACK DirectX11::WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
{
	switch (Msg)
	{
	case WM_KEYDOWN:
		DIRECTX.Key[wParam] = true;
		break;
	case WM_KEYUP:
		DIRECTX.Key[wParam] = false;
		break;
	case WM_DESTROY:
		DIRECTX.Running = false;
		break;
	default:
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	}
	if ((DIRECTX.Key['Q'] && DIRECTX.Key[VK_CONTROL]) || DIRECTX.Key[VK_ESCAPE])
	{
		DIRECTX.Running = false;
	}
	return 0;
}

DirectX11::DirectX11() :
	Window(nullptr),
	Running(false),
	WinSizeW(0),
	WinSizeH(0),
	Device(nullptr),
	Context(nullptr),
	SwapChain(nullptr),
	MainDepthBuffer(nullptr),
	BackBuffer(nullptr),
	BackBufferRT(nullptr),
	UniformBufferGen(nullptr),
	hInstance(nullptr)
{
	// Clear input
	for (int i = 0; i < sizeof(Key) / sizeof(Key[0]); ++i)
		Key[i] = false;
}

DirectX11::~DirectX11()
{
	ReleaseDevice();
	CloseWindow();
}

bool DirectX11::InitWindow(HINSTANCE hinst, LPCWSTR title)
{
	hInstance = hinst;
	Running = true;

	WNDCLASSW wc;
	memset(&wc, 0, sizeof(wc));
	wc.lpszClassName = L"App";
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = WindowProc;
	wc.cbWndExtra = sizeof(this);
	RegisterClassW(&wc);

	// adjust the window size and show at InitDevice time
	Window = CreateWindowW(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hinst, 0);
	if (!Window) return false;

	SetWindowLongPtr(Window, 0, LONG_PTR(this));

	return true;
}

void DirectX11::CloseWindow()
{
	if (Window)
	{
		DestroyWindow(Window);
		Window = nullptr;
		UnregisterClassW(L"App", hInstance);
	}
}

bool DirectX11::InitDevice(int vpW, int vpH, const LUID* pLuid, bool windowed, int scale)
{
	WinSizeW = vpW;
	WinSizeH = vpH;

	if (scale == 0)
		scale = 1;

	RECT size = { 0, 0, vpW / scale, vpH / scale };
	AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, false);
	const UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW;
	if (!SetWindowPos(Window, nullptr, 0, 0, size.right - size.left, size.bottom - size.top, flags))
		return false;

	IDXGIFactory * DXGIFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)(&DXGIFactory));
	VALIDATE((hr == ERROR_SUCCESS), "CreateDXGIFactory1 failed");

	IDXGIAdapter * Adapter = nullptr;
	for (UINT iAdapter = 0; DXGIFactory->EnumAdapters(iAdapter, &Adapter) != DXGI_ERROR_NOT_FOUND; ++iAdapter)
	{
		DXGI_ADAPTER_DESC adapterDesc;
		Adapter->GetDesc(&adapterDesc);
		if ((pLuid == nullptr) || memcmp(&adapterDesc.AdapterLuid, pLuid, sizeof(LUID)) == 0)
			break;
		Release(Adapter);
	}

	UINT deviceCreateFlags = 0;
#ifdef OVR_BUILD_DEBUG
	deviceCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	auto DriverType = Adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;
	hr = D3D11CreateDevice(Adapter, DriverType, 0, deviceCreateFlags, 0, 0, D3D11_SDK_VERSION, &Device, 0, &Context);
	Release(Adapter);
	VALIDATE((hr == ERROR_SUCCESS), "D3D11CreateDevice failed");

	// Create swap chain
	DXGI_SWAP_CHAIN_DESC scDesc;
	memset(&scDesc, 0, sizeof(scDesc));
	scDesc.BufferCount = 2;
	scDesc.BufferDesc.Width = WinSizeW;
	scDesc.BufferDesc.Height = WinSizeH;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.OutputWindow = Window;
	scDesc.SampleDesc.Count = 1;
	scDesc.Windowed = windowed;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;
	hr = DXGIFactory->CreateSwapChain(Device, &scDesc, &SwapChain);
	Release(DXGIFactory);
	VALIDATE((hr == ERROR_SUCCESS), "CreateSwapChain failed");

	// Create backbuffer
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer);
	hr = Device->CreateRenderTargetView(BackBuffer, nullptr, &BackBufferRT);
	VALIDATE((hr == ERROR_SUCCESS), "CreateRenderTargetView failed");

	// Main depth buffer
	MainDepthBuffer = new DepthBuffer(Device, WinSizeW, WinSizeH);
	Context->OMSetRenderTargets(1, &BackBufferRT, MainDepthBuffer->GetDepthStencilView());

	// Buffer for shader constants
	UniformBufferGen = new DataBuffer(Device, D3D11_BIND_CONSTANT_BUFFER, nullptr, UNIFORM_DATA_SIZE);
	ID3D11Buffer* constantBuffer = UniformBufferGen->GetBuffer();
	Context->VSSetConstantBuffers(0, 1, &constantBuffer);

	PixelShaderConstantBuffer = new DataBuffer(Device, D3D11_BIND_CONSTANT_BUFFER, nullptr, sizeof(PixelShaderLightData));
	ID3D11Buffer* psConstantBuffer = PixelShaderConstantBuffer->GetBuffer();
	Context->PSSetConstantBuffers(0, 1, &psConstantBuffer);

	// Set max frame latency to 1
	IDXGIDevice1* DXGIDevice1 = nullptr;
	hr = Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&DXGIDevice1);
	VALIDATE((hr == ERROR_SUCCESS), "QueryInterface failed");
	DXGIDevice1->SetMaximumFrameLatency(1);
	Release(DXGIDevice1);

	return true;
}

void DirectX11::SetAndClearRenderTarget(ID3D11RenderTargetView * rendertarget, DepthBuffer * depthbuffer, float R, float G, float B, float A)
{
	SetAndClearRenderTarget(rendertarget, depthbuffer ? depthbuffer->GetDepthStencilView() : (ID3D11DepthStencilView*)nullptr, R, G, B, A);
}

void DirectX11::SetAndClearRenderTarget(ID3D11RenderTargetView * rendertarget, ID3D11DepthStencilView* depthStencil,
	float R, float G, float B, float A)
{
	float black[] = { R, G, B, A }; // Important that alpha=0, if want pixels to be transparent, for manual layers
	Context->OMSetRenderTargets(1, &rendertarget, depthStencil);
	Context->ClearRenderTargetView(rendertarget, black);
	if (depthStencil)
		Context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}

void DirectX11::SetViewport(float vpX, float vpY, float vpW, float vpH)
{
	D3D11_VIEWPORT D3Dvp;
	D3Dvp.Width = vpW;    D3Dvp.Height = vpH;
	D3Dvp.MinDepth = 0;   D3Dvp.MaxDepth = 1;
	D3Dvp.TopLeftX = vpX; D3Dvp.TopLeftY = vpY;
	Context->RSSetViewports(1, &D3Dvp);
}

bool DirectX11::HandleMessages(void)
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	// This is to provide a means to terminate after a maximum number of frames
	// to facilitate automated testing
#ifdef MAX_FRAMES_ACTIVE 
	if (maxFrames > 0)
	{
		if (--maxFrames <= 0)
			Running = false;
	}
#endif
	return Running;
}

void DirectX11::Run(bool(*MainLoop)(bool retryCreate))
{
	while (HandleMessages())
	{
		// true => we'll attempt to retry for ovrError_DisplayLost
		if (!MainLoop(true))
			break;
		// Sleep a bit before retrying to reduce CPU load while the HMD is disconnected
		Sleep(10);
	}
}

void DirectX11::ReleaseDevice()
{
	Release(BackBuffer);
	Release(BackBufferRT);
	if (SwapChain)
	{
		SwapChain->SetFullscreenState(FALSE, nullptr);
		Release(SwapChain);
	}
	Release(Context);
	Release(Device);
	delete MainDepthBuffer;
	MainDepthBuffer = nullptr;
	delete UniformBufferGen;
	UniformBufferGen = nullptr;
}

DirectX11 DIRECTX;