#pragma once
#include "Vertex.h"
class TriangleSet
{
private:
	std::vector<Vertex> verts;
	std::vector<int> ind;
	Vertex* _vertices;
	int* _indices;
	bool changed;
	void update();
	void calcNormals();
public:
	TriangleSet();
	~TriangleSet();

	int numVertices();
	int numIndices();
	Vertex* Vertices();
	int* Indices();

	uint32_t ModifyColor(uint32_t c, XMFLOAT3 pos);

	// Non-indexed primitives
	void AddTriangle(Vertex v0, Vertex v1, Vertex v2);
	void AddQuad(Vertex v0, Vertex v1, Vertex v2, Vertex v3);
	void AddSolidColorBox(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t c);

	// Indexed primitives
	void SetVertex(Vertex v0, unsigned int index);
	void AddIndexedTriangle(int v0, int v1, int v2);
};

