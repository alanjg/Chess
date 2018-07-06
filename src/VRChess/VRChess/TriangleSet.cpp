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
	_indices = (short *)_aligned_malloc(ind.size() * sizeof(short), 16);
	for (unsigned int i = 0; i < verts.size(); i++)
	{
		_vertices[i] = verts[i];
	}
	for (unsigned int i = 0; i < ind.size(); i++)
	{
		_indices[i] = ind[i];
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
short* TriangleSet::Indices()
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
	for (int i = 0; i < 3; i++) ind.push_back(short(verts.size() + i));
	verts.push_back(v0);
	verts.push_back(v1);
	verts.push_back(v2);
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