#include "stdafx.h"
#include "VRChess.h"
#include "OculusTexture.h"
#include "Scene.h"
#include "Camera.h"
#include "Controller.h"
#include "COMHelpers.h"

VRChess::VRChess()
	:mirrorTexture(nullptr)
{	
	pEyeRenderTexture[0] = nullptr;
	pEyeRenderTexture[1] = nullptr;
}

VRChess::~VRChess()
{
	if (mirrorTexture)
	{
		ovr_DestroyMirrorTexture(session, mirrorTexture);
	}

	for (int eye = 0; eye < 2; ++eye)
	{
		delete pEyeRenderTexture[eye];
	}

	DIRECTX.ReleaseDevice();
	ovr_Destroy(session);
}

// return true to retry later (e.g. after display lost)
static bool MainLoop(bool retryCreate)
{
	VRChess vrChess;
	unique_ptr<Scene> roomScene;
	unique_ptr<Controller>controller;
	unique_ptr<Camera> mainCam;
	ovrMirrorTextureDesc mirrorDesc = {};
	long long frameIndex = 0;
	int msaaRate = 4;
	bool triggerState = false;
	ovrGraphicsLuid luid;
	ovrResult result = ovr_Create(&vrChess.session, &luid);
	if (!OVR_SUCCESS(result))
		return retryCreate;

	ovrHmdDesc hmdDesc = ovr_GetHmdDesc(vrChess.session);

	// Setup Device and Graphics
	// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
	if (!DIRECTX.InitDevice(hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2, reinterpret_cast<LUID*>(&luid)))
		goto Done;

	// Make the eye render buffers (caution if actual size < requested due to HW limits). 
	ovrRecti eyeRenderViewport[2];

	for (int eye = 0; eye < 2; ++eye)
	{
		ovrSizei idealSize = ovr_GetFovTextureSize(vrChess.session, (ovrEyeType)eye, hmdDesc.DefaultEyeFov[eye], 1.0f);
		vrChess.pEyeRenderTexture[eye] = new OculusTexture();
		if (!vrChess.pEyeRenderTexture[eye]->Init(vrChess.session, idealSize.w, idealSize.h, msaaRate, true))
		{
			if (retryCreate) goto Done;
			FATALERROR("Failed to create eye texture.");
		}
		eyeRenderViewport[eye].Pos.x = 0;
		eyeRenderViewport[eye].Pos.y = 0;
		eyeRenderViewport[eye].Size = idealSize;
		if (!vrChess.pEyeRenderTexture[eye]->GetTextureChain() || !vrChess.pEyeRenderTexture[eye]->GetDepthTextureChain())
		{
			if (retryCreate) goto Done;
			FATALERROR("Failed to create texture.");
		}
	}

	// Create a mirror to see on the monitor.
	mirrorDesc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
	mirrorDesc.Width = DIRECTX.WinSizeW;
	mirrorDesc.Height = DIRECTX.WinSizeH;
	mirrorDesc.MirrorOptions = ovrMirrorOption_Default;
	result = ovr_CreateMirrorTextureWithOptionsDX(vrChess.session, DIRECTX.Device, &mirrorDesc, &vrChess.mirrorTexture);

	if (!OVR_SUCCESS(result))
	{
		if (retryCreate) goto Done;
		FATALERROR("Failed to create mirror texture.");
	}

	// Create the room model
	roomScene.reset(new Scene());
	controller.reset(new Controller());
	// Create camera
	mainCam.reset(new Camera(XMVectorSet(-2.0f, 2.0f, 4.5f, 0), XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), -3.14159f/2.0f)));

	// FloorLevel will give tracking poses where the floor height is 0
	ovr_SetTrackingOriginType(vrChess.session, ovrTrackingOrigin_FloorLevel);
	
	// Main loop
	while (DIRECTX.HandleMessages())
	{
		ovrSessionStatus sessionStatus;
		ovr_GetSessionStatus(vrChess.session, &sessionStatus);
		if (sessionStatus.ShouldQuit)
		{
			// Because the application is requested to quit, should not request retry
			retryCreate = false;
			break;
		}
		if (sessionStatus.ShouldRecenter)
			ovr_RecenterTrackingOrigin(vrChess.session);

		if (sessionStatus.IsVisible)
		{
			XMVECTOR cameraPos = XMLoadFloat3(&mainCam->Posf);
			XMVECTOR cameraRot = XMLoadFloat4(&mainCam->Rotf);

			XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, -0.05f, 0), cameraRot);
			XMVECTOR right = XMVector3Rotate(XMVectorSet(0.05f, 0, 0, 0), cameraRot);
			if (DIRECTX.Key['W'] || DIRECTX.Key[VK_UP])      cameraPos = XMVectorAdd(cameraPos, forward);
			if (DIRECTX.Key['S'] || DIRECTX.Key[VK_DOWN])    cameraPos = XMVectorSubtract(cameraPos, forward);
			if (DIRECTX.Key['D'])                            cameraPos = XMVectorAdd(cameraPos, right);
			if (DIRECTX.Key['A'])                            cameraPos = XMVectorSubtract(cameraPos, right);
			static float Yaw = -3.14159f / 2.0f;
			if (DIRECTX.Key[VK_LEFT] || DIRECTX.Key['Q'])  cameraRot = XMQuaternionRotationRollPitchYaw(0, Yaw += 0.02f, 0);
			if (DIRECTX.Key[VK_RIGHT] || DIRECTX.Key['E']) cameraRot = XMQuaternionRotationRollPitchYaw(0, Yaw -= 0.02f, 0);

			XMStoreFloat3(&mainCam->Posf, cameraPos);
			XMStoreFloat4(&mainCam->Rotf, cameraRot);

			// Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
			ovrEyeRenderDesc eyeRenderDesc[2];
			eyeRenderDesc[0] = ovr_GetRenderDesc(vrChess.session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
			eyeRenderDesc[1] = ovr_GetRenderDesc(vrChess.session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

			// Get both eye poses simultaneously, with IPD offset already included. 
			ovrPosef EyeRenderPose[2];
			ovrPosef HmdToEyePose[2] = { eyeRenderDesc[0].HmdToEyePose, eyeRenderDesc[1].HmdToEyePose };

			double sensorSampleTime;    // sensorSampleTime is fed into the layer later
			ovr_GetEyePoses(vrChess.session, frameIndex, ovrTrue, HmdToEyePose, EyeRenderPose, &sensorSampleTime);

			double ftiming = ovr_GetPredictedDisplayTime(vrChess.session, 0);
			ovrTrackingState trackingState = ovr_GetTrackingState(vrChess.session, ftiming, ovrTrue);
			ovr_CalcEyePoses(trackingState.HeadPose.ThePose, HmdToEyePose, EyeRenderPose);

			ovrInputState inputState;
			if (ovr_GetInputState(vrChess.session, ovrControllerType_RTouch, &inputState) != ovrSuccess)
			{
				break;
			}
			bool hitTrigger = false;

			if (!triggerState)
			{
				if (inputState.IndexTrigger[ovrHand_Right] > 0.8f)
				{
					hitTrigger = true;
					triggerState = true;
				}
			}
			else
			{
				if (inputState.IndexTrigger[ovrHand_Right] < 0.2f)
				{
					triggerState = false;
				}
			}
			
			controller->UpdatePosition(trackingState, *mainCam);

			// Render Scene to Eye Buffers
			for (int eye = 0; eye < 2; ++eye)
			{
				// Clear and set up rendertarget
				DIRECTX.SetAndClearRenderTarget(vrChess.pEyeRenderTexture[eye]->GetRTV(), vrChess.pEyeRenderTexture[eye]->GetDSV());
				DIRECTX.SetViewport((float)eyeRenderViewport[eye].Pos.x, (float)eyeRenderViewport[eye].Pos.y,
					(float)eyeRenderViewport[eye].Size.w, (float)eyeRenderViewport[eye].Size.h);

				//Get the pose information in XM format
				XMVECTOR eyeQuat = XMVectorSet(EyeRenderPose[eye].Orientation.x, EyeRenderPose[eye].Orientation.y,
					EyeRenderPose[eye].Orientation.z, EyeRenderPose[eye].Orientation.w);
				XMVECTOR eyePos = XMVectorSet(EyeRenderPose[eye].Position.x, EyeRenderPose[eye].Position.y, EyeRenderPose[eye].Position.z, 0);

				// Get view and projection matrices for the Rift camera
				XMVECTOR CombinedPos = XMVectorAdd(cameraPos, XMVector3Rotate(eyePos, cameraRot));
				Camera finalCam(CombinedPos, XMQuaternionMultiply(eyeQuat, cameraRot));
				XMMATRIX view = finalCam.GetViewMatrix();
				ovrMatrix4f p = ovrMatrix4f_Projection(eyeRenderDesc[eye].Fov, 0.2f, 1000.0f, ovrProjection_None);
				vrChess.posTimewarpProjectionDesc[eye] = ovrTimewarpProjectionDesc_FromProjection(p, ovrProjection_None);
				XMMATRIX proj = XMMatrixSet(p.M[0][0], p.M[1][0], p.M[2][0], p.M[3][0],
					p.M[0][1], p.M[1][1], p.M[2][1], p.M[3][1],
					p.M[0][2], p.M[1][2], p.M[2][2], p.M[3][2],
					p.M[0][3], p.M[1][3], p.M[2][3], p.M[3][3]);
				XMMATRIX prod = XMMatrixMultiply(view, proj);
				XMVECTOR det;
				XMMATRIX viewInv = XMMatrixInverse(&det, view);
				XMMATRIX viewInvTranspose = XMMatrixTranspose(viewInv);				

				XMVECTOR dir = XMVectorSet(0, 0, -1, 0);
				
				XMVECTOR controllerPos = XMVectorSet(controller->GetModel()->Pos.x, controller->GetModel()->Pos.y, controller->GetModel()->Pos.z, 1.0);
				XMFLOAT4 controllerRot = controller->GetModel()->Rot;
				XMVECTOR quat = XMVectorSet(controllerRot.x, controllerRot.y, controllerRot.z, controllerRot.w);
				XMVECTOR pointing = XMVector3Rotate(dir, quat);
				XMVector3Normalize(pointing);
				XMFLOAT3 pointfx;
				XMStoreFloat3(&pointfx, pointing);

				float tmin;
				if (eye == 0)
				{
					Model* hit = roomScene->Pick(controller->GetModel()->Pos, pointfx, prod, tmin);
					if (hit != nullptr)
					{
						controller->UpdatePointer(tmin);
					}
					else
					{
						controller->UpdatePointer(1.0);
					}
					roomScene->UpdatePointer(controller->GetModel()->Pos, pointfx, prod);
					if (hitTrigger)
					{
						roomScene->HandlePointerClick(controller->GetModel()->Pos, pointfx, prod);
					}
				}

				PixelShaderLightData data;
				XMStoreFloat3(&data.eyePos, CombinedPos);
				data.lightDir.x = .707f;
				data.lightDir.y = -.707f;
				data.lightDir.z = 0;
				data.ambientLight = XMFLOAT4A(0.6f, 0.6f, 0.6f, 1.0f);
				data.diffuseLight = XMFLOAT4A(0.6f, 0.6f, 0.6f, 1.0f);
				data.specularLight = XMFLOAT4A(0.4f, 0.4f, 0.4f, 1.0f);
			

				// set pixel shader constant
				D3D11_MAPPED_SUBRESOURCE map;
				ID3D11Buffer* psConstantBuffer = DIRECTX.PixelShaderConstantBuffer->GetBuffer();
				DIRECTX.Context->Map(psConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
				memcpy(map.pData, &data, sizeof(PixelShaderLightData));
				DIRECTX.Context->Unmap(psConstantBuffer, 0);
				DIRECTX.Context->PSSetConstantBuffers(0, 1, &psConstantBuffer);

				roomScene->Update();
				roomScene->Render(&proj, &view);

				controller->GetModel()->Render(&proj, &view);
				controller->GetPointingModel()->Render(&proj, &view);

				// Commit rendering to the swap chain
				vrChess.pEyeRenderTexture[eye]->Commit();
			}

			// Initialize our single full screen Fov layer.
			ovrLayerEyeFovDepth ld = {};
			ld.Header.Type = ovrLayerType_EyeFovDepth;
			ld.Header.Flags = 0;

			for (int eye = 0; eye < 2; ++eye)
			{
				ld.ColorTexture[eye] = vrChess.pEyeRenderTexture[eye]->GetTextureChain();
				ld.DepthTexture[eye] = vrChess.pEyeRenderTexture[eye]->GetDepthTextureChain();
				ld.Viewport[eye] = eyeRenderViewport[eye];
				ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
				ld.RenderPose[eye] = EyeRenderPose[eye];
				ld.SensorSampleTime = sensorSampleTime;
				ld.ProjectionDesc = vrChess.posTimewarpProjectionDesc[eye];
			}

			ovrLayerHeader* layers = &ld.Header;
			result = ovr_SubmitFrame(vrChess.session, frameIndex, nullptr, &layers, 1);
			// exit the rendering loop if submit returns an error, will retry on ovrError_DisplayLost
			if (!OVR_SUCCESS(result))
				goto Done;

			frameIndex++;
		}

		// Render mirror
		ID3D11Texture2D* tex = nullptr;
		ovr_GetMirrorTextureBufferDX(vrChess.session, vrChess.mirrorTexture, IID_PPV_ARGS(&tex));

		DIRECTX.Context->CopyResource(DIRECTX.BackBuffer, tex);
		tex->Release();
		DIRECTX.SwapChain->Present(0, 0);

	}

	// Release resources
Done:

	// Retry on ovrError_DisplayLost
	return retryCreate || result == ovrError_DisplayLost;
}

//-------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hinst, HINSTANCE, LPSTR, int)
{
	// Initializes LibOVR, and the Rift
	ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
	ovrResult result = ovr_Initialize(&initParams);
	VALIDATE(OVR_SUCCESS(result), "Failed to initialize libOVR.");

	VALIDATE(DIRECTX.InitWindow(hinst, L"Chess"), "Failed to open window.");

	DIRECTX.Run(MainLoop);

	ovr_Shutdown();
	return(0);
}