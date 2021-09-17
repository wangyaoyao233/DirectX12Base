#pragma once

#include "main.h"

using namespace Microsoft::WRL;

class CPolygon
{
private:
	ComPtr<ID3D12Resource> m_VertexBuffer;
	ComPtr<ID3D12Resource> m_ConstantBuffer;

public:
	CPolygon();

	void Initialize();
	void Update();
	void Draw(ID3D12GraphicsCommandList* CommandList);
};