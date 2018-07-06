#pragma once
class Model;
class TriangleSet;
class Camera;
class Controller
{
	unique_ptr<Model> model, pointingModel;
public:
	Controller();
	void UpdatePosition(ovrTrackingState& trackingState, Camera& mainCam);
	void UpdatePointer(float depth);
	Model* GetModel();
	Model* GetPointingModel();
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetDirection();
};

