#include "stdafx.h"
#include "Controller.h"
#include "TriangleSet.h"
#include "Model.h"
#include "GeneratedTexture.h"
#include "Camera.h"

Controller::Controller()
{	
	TriangleSet* cube = new TriangleSet();
	cube->AddSolidColorBox(0.05f, -0.05f, 0.05f, -0.05f, 0.05f, -0.05f, 0xff404040);

	TriangleSet* cone = new TriangleSet();
	cone->AddSolidColorBox(0.01f, 0, 1, 0, .01f, 0, 0xff00ffff);

	model.reset(new Model(cube, XMFLOAT3(0, 0, 0), XMFLOAT4(0, 0, 0, 1), shared_ptr<Material>(new Material(new GeneratedTexture(false, 256, 256, GeneratedTexture::AUTO_CEILING)))));
	pointingModel.reset(new Model(cone, XMFLOAT3(0, 0, 0), XMFLOAT4(0, 0, 0, 1), shared_ptr<Material>(new Material(new GeneratedTexture(false, 256, 256, GeneratedTexture::AUTO_WHITE)))));
}

void Controller::UpdatePosition(ovrTrackingState& trackingState, Camera& mainCam)
{	
	XMVECTOR cameraPos = XMLoadFloat3(&mainCam.Posf);
	XMVECTOR cameraRot = XMLoadFloat4(&mainCam.Rotf);
	
	XMVECTOR handRot = XMVectorSet(trackingState.HandPoses[ovrHand_Right].ThePose.Orientation.x,
		trackingState.HandPoses[ovrHand_Right].ThePose.Orientation.y,
		trackingState.HandPoses[ovrHand_Right].ThePose.Orientation.z,
		trackingState.HandPoses[ovrHand_Right].ThePose.Orientation.w);
	XMVECTOR modelRot = XMQuaternionMultiply(handRot, cameraRot);
	XMVECTOR handPos = XMVectorSet(trackingState.HandPoses[ovrHand_Right].ThePose.Position.x, trackingState.HandPoses[ovrHand_Right].ThePose.Position.y, trackingState.HandPoses[ovrHand_Right].ThePose.Position.z, 1.0);
	
	XMVECTOR rotatedHandPos = XMVector3Rotate(handPos, cameraRot);
	XMVECTOR translatedHandPos = XMVectorAdd(cameraPos, rotatedHandPos);

	XMStoreFloat3(&model->Pos, translatedHandPos);
	XMStoreFloat4(&model->Rot, modelRot);
	
	XMStoreFloat3(&pointingModel->Pos, translatedHandPos);
	XMStoreFloat4(&pointingModel->Rot, modelRot);
}

Model* Controller::GetModel()
{
	return &(*model);
}

Model* Controller::GetPointingModel()
{
	return &(*pointingModel);
}

XMFLOAT3 Controller::GetPosition()
{
	return model->Pos;
}

XMFLOAT3 Controller::GetDirection()
{
	return model->Pos;	
}

void Controller::UpdatePointer(float depth)
{
	pointingModel->Scale.z = -depth;
}