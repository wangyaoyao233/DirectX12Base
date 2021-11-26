#pragma once


#include "main.h"
#include "renderer.h"
#include "model.h"

using namespace Microsoft::WRL;

class CSky
{
private:
	std::unique_ptr<CModel> m_Model;
	ComPtr<ID3D12Resource>	m_ConstantBuffer;

public:
	CSky();
	~CSky();

	void Initialize();
	void Update();
	void Draw(ID3D12GraphicsCommandList* CommandList);
};