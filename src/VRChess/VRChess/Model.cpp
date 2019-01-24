#include "stdafx.h"
#include "Model.h"
#include "DirectX11.h"
#include "Collision.h"

void Model::Init(TriangleSet * t)
{
	m_triangleSet = shared_ptr<TriangleSet>(t);
	m_numIndices = t->numIndices();
	m_vertexBuffer.reset(new DataBuffer(DIRECTX.Device, D3D11_BIND_VERTEX_BUFFER, &t->Vertices()[0], t->numVertices() * sizeof(Vertex)));
	m_indexBuffer.reset(new DataBuffer(DIRECTX.Device, D3D11_BIND_INDEX_BUFFER, &t->Indices()[0], t->numIndices() * sizeof(int)));
	Vertex* vertices = t->Vertices();
	ComputeBoundingAxisAlignedBoxFromPoints(&m_boundingBox, t->numVertices(), &vertices[0].Pos, sizeof(Vertex));
}

Model::Model(TriangleSet * t, XMFLOAT3 argPos, XMFLOAT4 argRot, shared_ptr<Material> argFill) :
	Pos(argPos),
	Rot(argRot),
	Scale(XMFLOAT3(1, 1, 1)),
	m_fill(argFill)
{
	Init(t);
}

void Model::Render(XMMATRIX * proj, XMMATRIX* view)
{	
	XMMATRIX modelMat = XMMatrixMultiply(XMMatrixScalingFromVector(XMLoadFloat3(&Scale)), XMMatrixMultiply(XMMatrixRotationQuaternion(XMLoadFloat4(&Rot)), XMMatrixTranslationFromVector(XMLoadFloat3(&Pos))));
	XMMATRIX modelView = XMMatrixMultiply(modelMat, *view);
	XMVECTOR det;
	XMMATRIX modelViewInv = XMMatrixInverse(&det, modelView);
	XMMATRIX modelInv = XMMatrixInverse(&det, modelMat);
	XMMATRIX viewInvTranspose = XMMatrixTranspose(modelViewInv);
	XMMATRIX modelInvTranspose = XMMatrixTranspose(modelInv);
	XMMATRIX modelViewProj = XMMatrixMultiply(modelView, *proj);
	
	float col[] = { 1, 1, 1, 1 };
	
	memcpy(DIRECTX.UniformData + 0, &modelViewProj, 64); // ProjView
	memcpy(DIRECTX.UniformData + 64, &modelMat, 64); // World
	memcpy(DIRECTX.UniformData + 64+64, &modelInvTranspose, 64); // WorldInvTranspose
	memcpy(DIRECTX.UniformData + 64+64+64, &col, 16); // MasterCol

	D3D11_MAPPED_SUBRESOURCE map;
	DIRECTX.Context->Map(DIRECTX.UniformBufferGen->GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	memcpy(map.pData, &DIRECTX.UniformData, DIRECTX.UNIFORM_DATA_SIZE);
	DIRECTX.Context->Unmap(DIRECTX.UniformBufferGen->GetBuffer(), 0);
	DIRECTX.Context->IASetInputLayout(m_fill->InputLayout);
	DIRECTX.Context->IASetIndexBuffer(m_indexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
	UINT offset = 0;
	ID3D11Buffer* vertexBuffer = m_vertexBuffer->GetBuffer();
	ID3D11Buffer* constantBuffer = m_fill->materialBuffer->GetBuffer();
	ID3D11Buffer* globalConstantBuffer = DIRECTX.PixelShaderConstantBuffer->GetBuffer();
	DIRECTX.Context->PSSetConstantBuffers(0, 1, &globalConstantBuffer);
	DIRECTX.Context->PSSetConstantBuffers(1, 1, &constantBuffer);

	DIRECTX.Context->IASetVertexBuffers(0, 1, &vertexBuffer, &m_fill->VertexSize, &offset);
	DIRECTX.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DIRECTX.Context->VSSetShader(m_fill->VertexShader, nullptr, 0);
	DIRECTX.Context->PSSetShader(m_fill->PixelShader, nullptr, 0);

	DIRECTX.Context->PSSetSamplers(0, 1, &m_fill->SamplerState);
	DIRECTX.Context->RSSetState(m_fill->Rasterizer);
	DIRECTX.Context->OMSetDepthStencilState(m_fill->DepthState, 0);
	DIRECTX.Context->OMSetBlendState(m_fill->BlendState, nullptr, 0xffffffff);
	ID3D11ShaderResourceView* shaderResources = m_fill->Tex->GetShaderResourceView();
	DIRECTX.Context->PSSetShaderResources(0, 1, &shaderResources);

	DIRECTX.Context->DrawIndexed((UINT)m_numIndices, 0, 0);
}

// render using stereo instancing
void Model::RenderInstanced(XMMATRIX *viewProjMats)
{
	XMMATRIX modelMat = XMMatrixMultiply(XMMatrixRotationQuaternion(XMLoadFloat4(&Rot)), XMMatrixTranslationFromVector(XMLoadFloat3(&Pos)));
	XMMATRIX modelViewProjMatL = XMMatrixMultiply(modelMat, viewProjMats[0]);
	XMMATRIX modelViewProjMatR = XMMatrixMultiply(modelMat, viewProjMats[1]);
	float col[] = { 1, 1, 1, 1 };
	
	memcpy(DIRECTX.UniformData + 0, &modelViewProjMatL, 64);
	memcpy(DIRECTX.UniformData + 64, &modelViewProjMatR, 64);
	memcpy(DIRECTX.UniformData + 128, &col, 16); // MasterCol
	D3D11_MAPPED_SUBRESOURCE map;
	DIRECTX.Context->Map(DIRECTX.UniformBufferGen->GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
	memcpy(map.pData, &DIRECTX.UniformData, DIRECTX.UNIFORM_DATA_SIZE);
	DIRECTX.Context->Unmap(DIRECTX.UniformBufferGen->GetBuffer(), 0);
	DIRECTX.Context->IASetInputLayout(m_fill->InputLayout);
	DIRECTX.Context->IASetIndexBuffer(m_indexBuffer->GetBuffer(), DXGI_FORMAT_R32_UINT, 0);
	UINT offset = 0;

	ID3D11Buffer* vertexBuffer = m_vertexBuffer->GetBuffer();
	ID3D11Buffer* constantBuffer = m_fill->materialBuffer->GetBuffer();
	ID3D11Buffer* globalConstantBuffer = DIRECTX.PixelShaderConstantBuffer->GetBuffer();
	DIRECTX.Context->PSSetConstantBuffers(0, 1, &globalConstantBuffer);
	DIRECTX.Context->PSSetConstantBuffers(1, 1, &constantBuffer);

	DIRECTX.Context->IASetVertexBuffers(0, 1, &vertexBuffer, &m_fill->VertexSize, &offset);
	DIRECTX.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	DIRECTX.Context->VSSetShader(m_fill->VertexShaderInstanced, nullptr, 0);
	DIRECTX.Context->PSSetShader(m_fill->PixelShader, nullptr, 0);
	DIRECTX.Context->PSSetSamplers(0, 1, &m_fill->SamplerState);
	DIRECTX.Context->RSSetState(m_fill->Rasterizer);
	DIRECTX.Context->OMSetDepthStencilState(m_fill->DepthState, 0);
	DIRECTX.Context->OMSetBlendState(m_fill->BlendState, nullptr, 0xffffffff);
	ID3D11ShaderResourceView* shaderResources = m_fill->Tex->GetShaderResourceView();
	DIRECTX.Context->PSSetShaderResources(0, 1, &shaderResources);

	// draw 2 instances
	DIRECTX.Context->DrawIndexedInstanced((UINT)m_numIndices, 2, 0, 0, 0);
}

bool Model::Pick(XMVECTOR rayOrigin, XMVECTOR rayDirection, float& tMin)
{
	// Assume we have not picked anything yet, so init to -1.
	int pickedTriangle = -1;
	float tmin = std::numeric_limits<float>::max();
	float tHit = tMin;
	if (XNA::IntersectRayAxisAlignedBox(rayOrigin, rayDirection, &m_boundingBox, &tHit))
	{
		for (int i = 0; i < m_triangleSet->numIndices(); i += 3)
		{
			XMVECTOR vFinal[3];
			for (int j = 0; j < 3; j++)
			{				
				Vertex& v = m_triangleSet->Vertices()[m_triangleSet->Indices()[i + j]];
				vFinal[j] = XMVectorSet(v.Pos.x, v.Pos.y, v.Pos.z, 1.0f);
			}
			float t = 0.0f;
			if (XNA::IntersectRayTriangle(rayOrigin, rayDirection, vFinal[0], vFinal[1], vFinal[2], &t))
			{
				if (t < tmin)
				{
					// This is the new nearest picked triangle.
					tmin = t;
					pickedTriangle = i;
				}
			}
		}
	}

	if (pickedTriangle != -1)
	{
		tMin = tmin;
		return true;
	}
	else
	{
		return false;
	}
}