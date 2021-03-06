#pragma once

#include "DirectX11.h"
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
class OculusTexture;

struct VRChess
{
	ovrMirrorTexture mirrorTexture;
	OculusTexture* pEyeRenderTexture[2];
	ovrTimewarpProjectionDesc posTimewarpProjectionDesc[ovrEye_Count];
	ovrSession session;

	VRChess();
	~VRChess();
};