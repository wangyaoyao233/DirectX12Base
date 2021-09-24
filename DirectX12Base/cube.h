#pragma once
#include "main.h"
#include "texture.h"

class CCube
{
private:
	ComPtr<ID3D12Resource>	m_VertexBuffer;
	ComPtr<ID3D12Resource>	m_IndexBuffer;
	ComPtr<ID3D12Resource>	m_ConstantBuffer;

	CTexture				m_Texture;
	XMFLOAT3				m_Rotation;
public:
	CCube();

	void Initialize();
	void Update();
	void Draw(ID3D12GraphicsCommandList* CommandList);
};