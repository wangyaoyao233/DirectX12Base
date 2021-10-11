#pragma once

#include "main.h"
#include "renderer.h"

using namespace Microsoft::WRL;

class CPolygonDeferred
{
private:
	ComPtr<ID3D12Resource>	m_VertexBuffer;
	ComPtr<ID3D12Resource>	m_ConstantBuffer;

	CTexture				m_Texture;

public:
	CPolygonDeferred();

	void Initialize();
	void Update();
	void Draw(ID3D12GraphicsCommandList* CommandList, ID3D12DescriptorHeap* DescriptorHeap);
};