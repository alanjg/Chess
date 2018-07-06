#include "stdafx.h"
#include "DataBuffer.h"
#include "COMHelpers.h"

DataBuffer::DataBuffer(ID3D11Device * Device, D3D11_BIND_FLAG use, const void* buffer, size_t size) 
	: m_size(size)
{
	D3D11_BUFFER_DESC desc;   
	memset(&desc, 0, sizeof(desc));
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.BindFlags = use;
	desc.ByteWidth = (unsigned)size;
	D3D11_SUBRESOURCE_DATA sr;
	sr.pSysMem = buffer;
	sr.SysMemPitch = sr.SysMemSlicePitch = 0;
	Device->CreateBuffer(&desc, buffer ? &sr : nullptr, &m_D3DBuffer);
}

DataBuffer::~DataBuffer()
{
	Release(m_D3DBuffer);
}

ID3D11Buffer* DataBuffer::GetBuffer()
{
	return m_D3DBuffer;
}