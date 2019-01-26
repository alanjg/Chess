#include "stdafx.h"
#include "Camera.h"

Camera::Camera() 
{
}

Camera::Camera(const XMVECTOR & pos, const XMVECTOR & rot)
{
	XMStoreFloat3(&Posf, pos);
	XMStoreFloat4(&Rotf, rot);
}

Camera::Camera(const XMFLOAT3& posf, const XMFLOAT4& rotf)
	: Posf(posf), Rotf(rotf)
{	
}

XMMATRIX Camera::GetViewMatrix()
{
	XMVECTOR Pos = XMLoadFloat3(&Posf);
	XMVECTOR Rot = XMLoadFloat4(&Rotf);
	XMVECTOR forward = XMVector3Rotate(XMVectorSet(0, 0, -1, 0), Rot);
	return XMMatrixLookAtRH(Pos, XMVectorAdd(Pos, forward), XMVector3Rotate(XMVectorSet(0, 1, 0, 0), Rot));
}