#pragma once

#include "main.h"
#include "renderer.h"
#include "model.h"

using namespace Microsoft::WRL;

class CPlayer
{
private:
	std::unique_ptr<CModel> m_Model;
	ComPtr<ID3D12Resource>	m_ConstantBuffer;

	XMFLOAT3				m_Rotation;

public:
	CPlayer();
	~CPlayer();

	void Initialize();
	void Update();
	void Draw(ID3D12GraphicsCommandList* CommandList);
};

