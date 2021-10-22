#include "main.h"
#include "renderer.h"
#include "cube.h"

CCube::CCube()
{
	Initialize();
}

void CCube::Initialize()
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
	resourceDesc.Width = sizeof(Vertex3D) * 4 * 6;
	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_VertexBuffer));
	assert(SUCCEEDED(hr));

	// インデックスバッファの作成
	resourceDesc.Width = sizeof(unsigned short) * 3 * 2 * 6;
	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_IndexBuffer));
	assert(SUCCEEDED(hr));

	// 定数バッファの作成
	resourceDesc.Width = 256;//定数バッファは256byteアライン
	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_ConstantBuffer));
	assert(SUCCEEDED(hr));

	// 頂点データの書き込み
	Vertex3D* buffer{};
	hr = m_VertexBuffer->Map(0, nullptr, (void**)&buffer);
	assert(SUCCEEDED(hr));

	{
		//+y
		buffer[0].Position = { -1.0f,1.0f,1.0f };
		buffer[1].Position = { 1.0f,1.0f,1.0f };
		buffer[2].Position = { -1.0f,1.0f,-1.0f };
		buffer[3].Position = { 1.0f,1.0f,-1.0f };
		buffer[0].Normal = { 0.0f,1.0f,0.0f };
		buffer[1].Normal = { 0.0f,1.0f,0.0f };
		buffer[2].Normal = { 0.0f,1.0f,0.0f };
		buffer[3].Normal = { 0.0f,1.0f,0.0f };
		buffer[0].TexCoord = { 0.0f,0.0f };
		buffer[1].TexCoord = { 0.0f,1.0f };
		buffer[2].TexCoord = { 1.0f,0.0f };
		buffer[3].TexCoord = { 1.0f,1.0f };

		//-y
		buffer[4].Position = { 1.0f,-1.0f,1.0f };
		buffer[5].Position = { -1.0f,-1.0f,1.0f };
		buffer[6].Position = { 1.0f,-1.0f,-1.0f };
		buffer[7].Position = { -1.0f,-1.0f,-1.0f };
		buffer[4].Normal = { 0.0f,-1.0f,0.0f };
		buffer[5].Normal = { 0.0f,-1.0f,0.0f };
		buffer[6].Normal = { 0.0f,-1.0f,0.0f };
		buffer[7].Normal = { 0.0f,-1.0f,0.0f };
		buffer[4].TexCoord = { 0.0f,0.0f };
		buffer[5].TexCoord = { 0.0f,1.0f };
		buffer[6].TexCoord = { 1.0f,0.0f };
		buffer[7].TexCoord = { 1.0f,1.0f };

		//+z
		buffer[12].Position = { 1.0f,1.0f,1.0f };
		buffer[13].Position = { -1.0f,1.0f,1.0f };
		buffer[14].Position = { 1.0f,-1.0f,1.0f };
		buffer[15].Position = { -1.0f,-1.0f,1.0f };
		buffer[12].Normal = { 0.0f,0.0f,1.0f };
		buffer[13].Normal = { 0.0f,0.0f,1.0f };
		buffer[14].Normal = { 0.0f,0.0f,1.0f };
		buffer[15].Normal = { 0.0f,0.0f,1.0f };
		buffer[12].TexCoord = { 0.0f,0.0f };
		buffer[13].TexCoord = { 0.0f,1.0f };
		buffer[14].TexCoord = { 1.0f,0.0f };
		buffer[15].TexCoord = { 1.0f,1.0f };

		//-z
		buffer[8].Position = { -1.0f,1.0f,-1.0f };
		buffer[9].Position = { 1.0f,1.0f,-1.0f };
		buffer[10].Position = { -1.0f,-1.0f,-1.0f };
		buffer[11].Position = { 1.0f,-1.0f,-1.0f };
		buffer[8].Normal = { 0.0f,0.0f,-1.0f };
		buffer[9].Normal = { 0.0f,0.0f,-1.0f };
		buffer[10].Normal = { 0.0f,0.0f,-1.0f };
		buffer[11].Normal = { 0.0f,0.0f,-1.0f };
		buffer[8].TexCoord = { 0.0f,0.0f };
		buffer[9].TexCoord = { 0.0f,1.0f };
		buffer[10].TexCoord = { 1.0f,0.0f };
		buffer[11].TexCoord = { 1.0f,1.0f };

		//+x
		buffer[16].Position = { 1.0f,1.0f,-1.0f };
		buffer[17].Position = { 1.0f,1.0f,1.0f };
		buffer[18].Position = { 1.0f,-1.0f,-1.0f };
		buffer[19].Position = { 1.0f,-1.0f,1.0f };
		buffer[16].Normal = { 1.0f,0.0f,0.0f };
		buffer[17].Normal = { 1.0f,0.0f,0.0f };
		buffer[18].Normal = { 1.0f,0.0f,0.0f };
		buffer[19].Normal = { 1.0f,0.0f,0.0f };
		buffer[16].TexCoord = { 0.0f,0.0f };
		buffer[17].TexCoord = { 0.0f,1.0f };
		buffer[18].TexCoord = { 1.0f,0.0f };
		buffer[19].TexCoord = { 1.0f,1.0f };

		//-x
		buffer[20].Position = { -1.0f,1.0f,1.0f };
		buffer[21].Position = { -1.0f,1.0f,-1.0f };
		buffer[22].Position = { -1.0f,-1.0f,1.0f };
		buffer[23].Position = { -1.0f,-1.0f,-1.0f };
		buffer[20].Normal = { -1.0f,0.0f,0.0f };
		buffer[21].Normal = { -1.0f,0.0f,0.0f };
		buffer[22].Normal = { -1.0f,0.0f,0.0f };
		buffer[23].Normal = { -1.0f,0.0f,0.0f };
		buffer[20].TexCoord = { 0.0f,0.0f };
		buffer[21].TexCoord = { 0.0f,1.0f };
		buffer[22].TexCoord = { 1.0f,0.0f };
		buffer[23].TexCoord = { 1.0f,1.0f };
	}

	m_VertexBuffer->Unmap(0, nullptr);

	// インデックスデータの書き込み
	unsigned short* index;
	hr = m_IndexBuffer->Map(0, nullptr, (void**)&index);
	assert(SUCCEEDED(hr));

	{
		for (unsigned short i = 0; i < 6; ++i)
		{
			index[i * 6 + 0] = i * 4 + 0;
			index[i * 6 + 1] = i * 4 + 1;
			index[i * 6 + 2] = i * 4 + 2;

			index[i * 6 + 3] = i * 4 + 1;
			index[i * 6 + 4] = i * 4 + 3;
			index[i * 6 + 5] = i * 4 + 2;
		}
	}

	m_ConstantBuffer->Unmap(0, nullptr);

	// テクスチャ読み込み
	m_Texture.Load("asset/field004.tga");

	m_Rotation = { 0.0f,0.0f,0.0f };
}

void CCube::Update()
{
	m_Rotation.x += 0.01f;
	m_Rotation.y += 0.01f;
	m_Rotation.z += 0.01f;
}

void CCube::Draw(ID3D12GraphicsCommandList* CommandList)
{
	HRESULT hr;

	//マトリクス設定
	//XMMATRIX view = XMMatrixLookAtLH({ 0.0,2.0,-5.0 }, { 0.0,0.0,0.0 }, { 0.0,1.0,0.0 });
	//XMMATRIX projection = XMMatrixPerspectiveFovLH(1.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 1.0f, 20.0f);
	XMMATRIX view = CRenderer::GetInstance()->GetCamera3D()->GetViewMatrix();
	XMMATRIX projection = CRenderer::GetInstance()->GetCamera3D()->GetProjectionMatrix();

	XMMATRIX world = XMMatrixIdentity();

	world *= XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	world *= XMMatrixTranslation(0.0f, 1.0f, 0.0f);

	//定数バッファ設定
	Constant* constant;
	hr = m_ConstantBuffer->Map(0, nullptr, (void**)&constant);
	assert(SUCCEEDED(hr));

	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, XMMatrixTranspose(world * view * projection));
	constant->WVP = matrix;
	XMStoreFloat4x4(&matrix, XMMatrixTranspose(world));
	constant->World = matrix;

	//Light
	//constant->LightDirection = CRenderer::GetInstance()->GetLight()->GetDirection();
	//XMFLOAT3 cameraPos = CRenderer::GetInstance()->GetCamera3D()->GetCameraPostion();
	//constant->CameraPostion = { cameraPos.x, cameraPos.y, cameraPos.z ,0.0f };

	m_ConstantBuffer->Unmap(0, nullptr);

	CommandList->SetGraphicsRootConstantBufferView(0, m_ConstantBuffer->GetGPUVirtualAddress());

	//頂点バッファ設定
	D3D12_VERTEX_BUFFER_VIEW vertexView{};
	vertexView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	vertexView.StrideInBytes = sizeof(Vertex3D);
	vertexView.SizeInBytes = sizeof(Vertex3D) * 4 * 6;
	CommandList->IASetVertexBuffers(0, 1, &vertexView);

	//インデックスバッファ設定
	D3D12_INDEX_BUFFER_VIEW indexView{};
	indexView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	indexView.Format = DXGI_FORMAT_R16_UINT;
	indexView.SizeInBytes = sizeof(unsigned short) * 3 * 2 * 6;
	CommandList->IASetIndexBuffer(&indexView);

	//テクスチャ設定
	ID3D12DescriptorHeap* dh[] = { *m_Texture.GetDescriptorHeap().GetAddressOf() };
	CommandList->SetDescriptorHeaps(_countof(dh), dh);
	CommandList->SetGraphicsRootDescriptorTable(1, m_Texture.GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

	//トボロジ設定
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//描画
	CommandList->DrawIndexedInstanced(36, 1, 0, 0, 0);
}