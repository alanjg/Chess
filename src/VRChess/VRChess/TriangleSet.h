#pragma once
#include "Vertex.h"
class TriangleSet
{
private:
	std::vector<Vertex> verts;
	std::vector<short> ind;
	Vertex* _vertices;
	short* _indices;
	bool changed;
	void update();
public:
	TriangleSet();
	~TriangleSet();

	int numVertices();
	int numIndices();
	Vertex* Vertices();
	short* Indices();

	void AddSolidColorBox(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t c);
	uint32_t ModifyColor(uint32_t c, XMFLOAT3 pos);
	void AddTriangle(Vertex v0, Vertex v1, Vertex v2);
	void AddQuad(Vertex v0, Vertex v1, Vertex v2, Vertex v3);
};

