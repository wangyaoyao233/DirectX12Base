#pragma once

#include "main.h"

using namespace Microsoft::WRL;

class CTexture
{
private:
	ComPtr<ID3D12Resource>			m_Resource;
	ComPtr<ID3D12DescriptorHeap>	m_DescriptorHeap;

	void TGALoad(const char* FileName, std::vector<char>& image, unsigned int& width, unsigned int& height);
public:

	void Load(const char* FileName);

	ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap()
	{
		return m_DescriptorHeap;
	}
};