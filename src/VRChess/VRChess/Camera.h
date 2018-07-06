#pragma once
class Camera
{
public:
	XMFLOAT3 Posf;
	XMFLOAT4 Rotf;

	Camera();
	Camera(const XMFLOAT3& posf, const XMFLOAT4& rotf);
	Camera(const XMVECTOR& pos, const XMVECTOR& rot);

	XMMATRIX GetViewMatrix();
};

