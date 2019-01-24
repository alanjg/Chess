#include "stdafx.h"
#include "TriangleSet.h"
#include "COMHelpers.h"

TriangleSet::TriangleSet()
{
	_vertices = nullptr;
	_indices = nullptr;
	changed = true;
}

TriangleSet::~TriangleSet()
{
	if (_vertices != nullptr)
	{
		_aligned_free(_vertices);
		_aligned_free(_indices);

		_vertices = nullptr;
		_indices = nullptr;
	}
}
int TriangleSet::numVertices()
{
	return verts.size();
}

void TriangleSet::update()
{
	if (_vertices != nullptr)
	{
		_aligned_free(_vertices);
		_aligned_free(_indices);
	}
	_vertices = (Vertex *)_aligned_malloc(verts.size() * sizeof(Vertex), 16);
	_indices = (int *)_aligned_malloc(ind.size() * sizeof(int), 16);
	calcNormals();
	for (unsigned int i = 0; i < verts.size(); i++)
	{
		_vertices[i] = verts[i];
	}
	for (unsigned int i = 0; i < ind.size(); i++)
	{
		_indices[i] = ind[i];
	}
}

void TriangleSet::calcNormals()
{
	vector<vector<XMFLOAT3>> normals(verts.size());
	for (unsigned int i = 0; i < ind.size(); i += 3)
	{
		XMVECTOR p1 = XMLoadFloat3(&verts[ind[i + 0]].Pos);
		XMVECTOR p2 = XMLoadFloat3(&verts[ind[i + 1]].Pos);
		XMVECTOR p3 = XMLoadFloat3(&verts[ind[i + 2]].Pos);
		XMVECTOR normal = XMVector3Cross(p1 - p3, p1 - p2);
		XMVECTOR norm = XMVector3Normalize(normal);
		XMFLOAT3 normf;
		XMStoreFloat3(&normf, norm);
		normals[ind[i + 0]].push_back(normf);
		normals[ind[i + 1]].push_back(normf);
		normals[ind[i + 2]].push_back(normf);
	}
	for (unsigned int i = 0; i < normals.size();i++)
	{
		XMVECTOR acc = XMVectorSet(0, 0, 0, 0);
		float count = 0;
		for (unsigned int j=0;j<normals[i].size(); j++)
		{			
			acc = XMVectorAdd(acc, XMLoadFloat3(&normals[i][j]));
			count += 1.0f;
		}
		if (count > 0.0f)
		{
			XMVECTOR res = XMVectorDivide(acc, XMVectorSet(count, count, count, 1.0));
			XMStoreFloat3(&verts[i].Norm, XMVector3Normalize(res));
		}
		else
		{
			XMStoreFloat3(&verts[i].Norm, acc);
		}
	}
}

int TriangleSet::numIndices()
{
	return ind.size();
}

Vertex* TriangleSet::Vertices()
{
	if (changed)
	{
		update();
		changed = false;
	}
	return _vertices;
}

int* TriangleSet::Indices()
{
	if (changed)
	{
		update();
		changed = false;
	}
	return _indices;
}

void TriangleSet::AddQuad(Vertex v0, Vertex v1, Vertex v2, Vertex v3) 
{ 
	AddTriangle(v0, v1, v2);	
	AddTriangle(v3, v2, v1); 
}

void TriangleSet::AddTriangle(Vertex v0, Vertex v1, Vertex v2)
{
	for (int i = 0; i < 3; i++) ind.push_back(verts.size() + i);
	verts.push_back(v0);
	verts.push_back(v1);
	verts.push_back(v2);
	changed = true;
}

void TriangleSet::SetVertex(Vertex v0, unsigned int index)
{
	if (index >= verts.size()) verts.resize(index + 1);
	verts[index] = v0;
	changed = true;
}

void TriangleSet::AddIndexedTriangle(int v0, int v1, int v2)
{
	ind.push_back(v0);
	ind.push_back(v1);
	ind.push_back(v2);
	changed = true;
}

uint32_t TriangleSet::ModifyColor(uint32_t c, XMFLOAT3 pos)
{
#define GetLengthLocal(v)  (sqrt(v.x*v.x + v.y*v.y + v.z*v.z))
	float dist1 = GetLengthLocal(XMFLOAT3(pos.x - (-2), pos.y - (4), pos.z - (-2)));
	float dist2 = GetLengthLocal(XMFLOAT3(pos.x - (3), pos.y - (4), pos.z - (-3)));
	float dist3 = GetLengthLocal(XMFLOAT3(pos.x - (-4), pos.y - (3), pos.z - (25)));
	int   bri = rand() % 160;
	float R = ((c >> 16) & 0xff) * (bri + 192.0f*(0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
	float G = ((c >> 8) & 0xff) * (bri + 192.0f*(0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
	float B = ((c >> 0) & 0xff) * (bri + 192.0f*(0.65f + 8 / dist1 + 1 / dist2 + 4 / dist3)) / 255.0f;
	return((c & 0xff000000) + ((R>255 ? 255 : (uint32_t)R) << 16) + ((G>255 ? 255 : (uint32_t)G) << 8) + (B>255 ? 255 : (uint32_t)B));
}

void TriangleSet::AddSolidColorBox(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t c)
{
	AddQuad(Vertex(XMFLOAT3(x1, y2, z1), ModifyColor(c, XMFLOAT3(x1, y2, z1)), z1, x1),
		Vertex(XMFLOAT3(x2, y2, z1), ModifyColor(c, XMFLOAT3(x2, y2, z1)), z1, x2),
		Vertex(XMFLOAT3(x1, y2, z2), ModifyColor(c, XMFLOAT3(x1, y2, z2)), z2, x1),
		Vertex(XMFLOAT3(x2, y2, z2), ModifyColor(c, XMFLOAT3(x2, y2, z2)), z2, x2));
	AddQuad(Vertex(XMFLOAT3(x2, y1, z1), ModifyColor(c, XMFLOAT3(x2, y1, z1)), z1, x2),
		Vertex(XMFLOAT3(x1, y1, z1), ModifyColor(c, XMFLOAT3(x1, y1, z1)), z1, x1),
		Vertex(XMFLOAT3(x2, y1, z2), ModifyColor(c, XMFLOAT3(x2, y1, z2)), z2, x2),
		Vertex(XMFLOAT3(x1, y1, z2), ModifyColor(c, XMFLOAT3(x1, y1, z2)), z2, x1));
	AddQuad(Vertex(XMFLOAT3(x1, y1, z2), ModifyColor(c, XMFLOAT3(x1, y1, z2)), z2, y1),
		Vertex(XMFLOAT3(x1, y1, z1), ModifyColor(c, XMFLOAT3(x1, y1, z1)), z1, y1),
		Vertex(XMFLOAT3(x1, y2, z2), ModifyColor(c, XMFLOAT3(x1, y2, z2)), z2, y2),
		Vertex(XMFLOAT3(x1, y2, z1), ModifyColor(c, XMFLOAT3(x1, y2, z1)), z1, y2));
	AddQuad(Vertex(XMFLOAT3(x2, y1, z1), ModifyColor(c, XMFLOAT3(x2, y1, z1)), z1, y1),
		Vertex(XMFLOAT3(x2, y1, z2), ModifyColor(c, XMFLOAT3(x2, y1, z2)), z2, y1),
		Vertex(XMFLOAT3(x2, y2, z1), ModifyColor(c, XMFLOAT3(x2, y2, z1)), z1, y2),
		Vertex(XMFLOAT3(x2, y2, z2), ModifyColor(c, XMFLOAT3(x2, y2, z2)), z2, y2));
	AddQuad(Vertex(XMFLOAT3(x1, y1, z1), ModifyColor(c, XMFLOAT3(x1, y1, z1)), x1, y1),
		Vertex(XMFLOAT3(x2, y1, z1), ModifyColor(c, XMFLOAT3(x2, y1, z1)), x2, y1),
		Vertex(XMFLOAT3(x1, y2, z1), ModifyColor(c, XMFLOAT3(x1, y2, z1)), x1, y2),
		Vertex(XMFLOAT3(x2, y2, z1), ModifyColor(c, XMFLOAT3(x2, y2, z1)), x2, y2));
	AddQuad(Vertex(XMFLOAT3(x2, y1, z2), ModifyColor(c, XMFLOAT3(x2, y1, z2)), x2, y1),
		Vertex(XMFLOAT3(x1, y1, z2), ModifyColor(c, XMFLOAT3(x1, y1, z2)), x1, y1),
		Vertex(XMFLOAT3(x2, y2, z2), ModifyColor(c, XMFLOAT3(x2, y2, z2)), x2, y2),
		Vertex(XMFLOAT3(x1, y2, z2), ModifyColor(c, XMFLOAT3(x1, y2, z2)), x1, y2));
}