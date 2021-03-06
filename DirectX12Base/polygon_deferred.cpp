#include "main.h"
#include "renderer.h"
#include "polygon_deferred.h"
#include "input.h"

CPolygonDeferred::CPolygonDeferred()
{
	Initialize();
}

void CPolygonDeferred::Initialize()
{
	ComPtr<ID3D12Device> device = CRenderer::GetInstance()->GetDevice();

	HRESULT hr{};
	D3D12_HEAP_PROPERTIES	heapProperties{};
	D3D12_RESOURCE_DESC		resourceDesc{};

	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProperties.CreationNodeMask = 0;
	heapProperties.VisibleNodeMask = 0;

	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;

	// 頂点バッファの作成
	resourceDesc.Width = sizeof(Vertex3D) * 4;
	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexBuffer));
	assert(SUCCEEDED(hr));

	// 定数バッファの作成
	resourceDesc.Width = 256;//定数バッファは256byteアライン
	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_ConstantBuffer));
	assert(SUCCEEDED(hr));

	// 頂点データの書き込み
	Vertex3D* buffer{};
	hr = m_VertexBuffer->Map(0, nullptr, (void**)&buffer);
	assert(SUCCEEDED(hr));

	buffer[0].Position = { 0.0f,0.0f,0.0f };
	buffer[1].Position = { SCREEN_WIDTH,0.0f,0.0f };
	buffer[2].Position = { 0.0f,SCREEN_HEIGHT,0.0f };
	buffer[3].Position = { SCREEN_WIDTH,SCREEN_HEIGHT,0.0f };
	buffer[0].Normal = { 0.0f,1.0f,0.0f };
	buffer[1].Normal = { 0.0f,1.0f,0.0f };
	buffer[2].Normal = { 0.0f,1.0f,0.0f };
	buffer[3].Normal = { 0.0f,1.0f,0.0f };
	buffer[0].TexCoord = { 0.0f,0.0f };
	buffer[1].TexCoord = { 1.0f,0.0f };
	buffer[2].TexCoord = { 0.0f,1.0f };
	buffer[3].TexCoord = { 1.0f,1.0f };

	m_VertexBuffer->Unmap(0, nullptr);

	m_Texture.Load("asset/field004.tga");


	/*
	float roughness = 0.0;
	float metallic = 1.0;
	float spec = 0.9;
	*/
	m_Param = { 0.0f, 1.0f, 0.9f, 0.0f };
}

void CPolygonDeferred::Update()
{
	//roughness
	if (CInput::GetKeyTrigger('Q'))
	{
		if (m_Param.x > 0.0f)
		{
			m_Param.x -= 0.1f;
		}
	}
	if (CInput::GetKeyTrigger('W'))
	{
		if (m_Param.x < 1.0f)
		{
			m_Param.x += 0.1f;
		}
	}
	//metallic
	if (CInput::GetKeyTrigger('A'))
	{
		if (m_Param.y > 0.0f)
		{
			m_Param.y -= 0.1f;
		}
	}
	if (CInput::GetKeyTrigger('S'))
	{
		if (m_Param.z < 1.0f)
		{
			m_Param.z += 0.1f;
		}
	}
	//spec
	if (CInput::GetKeyTrigger('Z'))
	{
		if (m_Param.z > 0.0f)
		{
			m_Param.z -= 0.1f;
		}
	}
	if (CInput::GetKeyTrigger('X'))
	{
		if (m_Param.z < 1.0f)
		{
			m_Param.z += 0.1f;
		}
	}

	if (CInput::GetKeyTrigger('P'))
	{
		m_Param.w = 1.0 - m_Param.w;
	}
}

void CPolygonDeferred::Draw(ID3D12GraphicsCommandList* CommandList, ID3D12DescriptorHeap* Texture)
{
	HRESULT hr;

	//マトリクス設定
	//XMMATRIX view = XMMatrixIdentity();
	//XMMATRIX projection = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
	XMMATRIX view = CRenderer::GetInstance()->GetCamera2D()->GetViewMatrix();
	XMMATRIX projection = CRenderer::GetInstance()->GetCamera2D()->GetProjectionMatrix();

	XMMATRIX world = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	//定数バッファ設定
	Constant* constant;
	hr = m_ConstantBuffer->Map(0, nullptr, (void**)&constant);
	assert(SUCCEEDED(hr));

	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, XMMatrixTranspose(world * view * projection));
	constant->WVP = matrix;
	XMStoreFloat4x4(&matrix, XMMatrixTranspose(world));
	constant->World = matrix;

	//TODO: 改用2d专用的shader, 不需要光照相关处理
	//Light
	//constant->LightDirection = { 0, -1, 0, 0 };
	//constant->CameraPostion = { 0,0,0,0 };

	//ImGUI
	{
		ImGui::Begin("Param set");

		ImGui::SliderFloat("roughness", &m_Param.x, 0.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("metallic", &m_Param.y, 0.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("spec", &m_Param.z, 0.0f, 1.0f, "%.1f");
		ImGui::SliderFloat("w", &m_Param.w, 0.0f, 1.0f, "%.1f");

		ImGui::End();
	}


	constant->Param = m_Param;

	m_ConstantBuffer->Unmap(0, nullptr);

	CommandList->SetGraphicsRootConstantBufferView(0, m_ConstantBuffer->GetGPUVirtualAddress());

	//頂点バッファ設定
	D3D12_VERTEX_BUFFER_VIEW vertexView{};
	vertexView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	vertexView.StrideInBytes = sizeof(Vertex3D);
	vertexView.SizeInBytes = sizeof(Vertex3D) * 4;
	CommandList->IASetVertexBuffers(0, 1, &vertexView);

	//テクスチャ設定
	ID3D12DescriptorHeap* dh[] = { Texture };
	CommandList->SetDescriptorHeaps(_countof(dh), dh);
	CommandList->SetGraphicsRootDescriptorTable(1, Texture->GetGPUDescriptorHandleForHeapStart());

	//ID3D12DescriptorHeap* dh[] = { *m_Texture.GetDescriptorHeap().GetAddressOf() };
	//CommandList->SetDescriptorHeaps(_countof(dh), dh);
	//CommandList->SetGraphicsRootDescriptorTable(1, m_Texture.GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

	//トボロジ設定
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//描画
	CommandList->DrawInstanced(4, 1, 0, 0);
}