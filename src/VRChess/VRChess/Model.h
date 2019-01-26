#pragma once
#include "TriangleSet.h"
#include "Material.h"
#include "DataBuffer.h"
#include "Collision.h"

class Model
{
private:
	XNA::AxisAlignedBox m_boundingBox;	
	shared_ptr<Material> m_fill;
	unique_ptr<DataBuffer> m_vertexBuffer;
	unique_ptr<DataBuffer> m_indexBuffer;
	int m_numIndices;
	shared_ptr<TriangleSet> m_triangleSet;

	void Init(TriangleSet * t);
public:
	Model(TriangleSet * t, XMFLOAT3 argPos, XMFLOAT4 argRot, shared_ptr<Material> argFill);
	void Render(XMMATRIX * proj, XMMATRIX* view);
	bool Pick(XMVECTOR rayOrigin, XMVECTOR rayDirection, float& tMin);

	XMFLOAT3 Pos;
	XMFLOAT4 Rot;
	XMFLOAT3 Scale;
};

