#pragma once
class DataBuffer
{	
	size_t m_size;
	ID3D11Buffer* m_D3DBuffer;
public:
	DataBuffer(ID3D11Device * Device, D3D11_BIND_FLAG use, const void* buffer, size_t size);
	~DataBuffer();
	ID3D11Buffer* GetBuffer();
};

