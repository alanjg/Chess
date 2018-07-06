#pragma once
struct Vertex
{
	XMFLOAT3  Pos;
	uint32_t  C;
	float     U, V;
	Vertex() {};
	Vertex(XMFLOAT3 pos, uint32_t c, float u, float v) : Pos(pos), C(c), U(u), V(v) {};
};