#pragma once
struct Vertex
{
	XMFLOAT3  Pos;
	XMFLOAT3  Norm;
	uint32_t  Color;
	float     U, V;
	Vertex() {};
	Vertex(XMFLOAT3 pos, uint32_t c, float u, float v) : Pos(pos), Color(c), U(u), V(v) {};
	Vertex(XMFLOAT3 pos, XMFLOAT3 norm, uint32_t c, float u, float v) : Pos(pos), Norm(norm), Color(c), U(u), V(v) {};
};