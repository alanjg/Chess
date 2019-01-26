#include "stdafx.h"
#include "MeshLoader.h"
#include "Model.h"
#include "TriangleSet.h"
#include "Material.h"
#include "ImageTexture.h"

MeshLoader::MeshLoader()
{
}


MeshLoader::~MeshLoader()
{
}

Model* MeshLoader::LoadMeshFromFile(const std::string& file)
{
	TriangleSet* triangleSet = new TriangleSet();
	XMFLOAT3 pos = XMFLOAT3(0, 0, 0);
	XMFLOAT4 rot = XMFLOAT4(0, 0, 0, 1);
	wchar_t result[MAX_PATH];
	std::wstring path(result, GetModuleFileName(NULL, result, MAX_PATH));
	int index = path.find_last_of('\\');
	path = path.substr(0, index + 1);
	std::wstring meshFile = path + std::wstring(file.begin(), file.end());
	std::ifstream in(meshFile);
	std::string word;
	in >> word;
	MaterialData materialData;
	if (word != "Material")
	{
		return false;
	}
	in >> word;
	Texture* texture;
	uint32_t dcolor = 0xffffffff, scolor = 0xffffffff;
	uint8_t spower = 0;
	if (word == "Diffuse")
	{
		in >> materialData.diffuse.x >> materialData.diffuse.y >> materialData.diffuse.z >> materialData.diffuse.w;
		materialData.ambient = materialData.diffuse;
		in >> word;
	}
	if (word == "Specular")
	{
		in >> materialData.specular.x >> materialData.specular.y >> materialData.specular.z >> materialData.specular.w >> materialData.specPower;
		in >> word;
	}
	
	if (word == "Texture")
	{
		std::string url;
		std::getline(in, url);
		// trim leading whitespace
		size_t firstNonSpace = 0;
		while (firstNonSpace < url.size() && isspace(url[firstNonSpace])) firstNonSpace++;
		url.erase(url.begin(), url.begin() + firstNonSpace);
		wstring imageFile = path + wstring(url.begin(), url.end());
		ImageTexture* it = new ImageTexture();
		if (!it->Init(imageFile))
		{
			return false;
		}
		texture = it;
		in >> word;
	}
	else
	{
		texture = new GeneratedTexture(false, 255, 255, GeneratedTexture::AUTO_WHITE);
	}

	if (word != "Positions")
	{
		return false;
	}
	int count;
	in >> count;
	std::vector<float> vvx, vvy, vvz;
	for (int i = 0; i < count; i++)
	{
		float vx, vy, vz;
		in >> vx >> vy >> vz;
		vvx.push_back(vx);
		vvy.push_back(vy);
		vvz.push_back(vz);
	}
	in >> word;
	if (word != "Indexes")
	{
		return false;
	}
	std::vector<int> indexes;
	in >> count;
	for (int i = 0; i < count; i++)
	{
		int ind;
		in >> ind;
		indexes.push_back(ind);
	}
	in >> word;
	if (word != "TexCoords")
	{
		return false;
	}
	in >> count;
	std::vector<float> tu, tv;
	for (int i = 0; i < count; i++)
	{
		float vx, vy;
		in >> vx >> vy;
		tu.push_back(vx);
		tv.push_back(vy);
	}
	for (unsigned int i = 0; i < vvx.size(); i++)
	{
		triangleSet->SetVertex(Vertex(XMFLOAT3(vvx[i], vvy[i], vvz[i]), dcolor, tu[i], tv[i]), i);
	}
	for (unsigned int i = 0; i < indexes.size(); i += 3)
	{
		triangleSet->AddIndexedTriangle(indexes[i], indexes[i+1], indexes[i+2]);
	}
	
	shared_ptr<Material> material(new Material(texture, &materialData));
	Model* model = new Model(triangleSet, pos, rot, material);
	return model;
}