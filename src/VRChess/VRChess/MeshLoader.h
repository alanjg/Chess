#pragma once
#include "Model.h"
class MeshLoader
{
public:
	MeshLoader();
	~MeshLoader();

	Model* LoadMeshFromFile(const std::string& file);
};

