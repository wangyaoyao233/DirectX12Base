#include "main.h"
#include "renderer.h"
#include "field.h"

CField::CField()
{
	Initialize();
}

void CField::Initialize()
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

	buffer[0].Position = { -10.0f,0.0f,10.0f };
	buffer[1].Position = { 10.0f,0.0f,10.0f };
	buffer[2].Position = { -10.0f,0.0f,-10.0f };
	buffer[3].Position = { 10.0f,0.0f,-10.0f };
	buffer[0].Normal = { 0.0f,1.0f,0.0f };
	buffer[1].Normal = { 0.0f,1.0f,0.0f };
	buffer[2].Normal = { 0.0f,1.0f,0.0f };
	buffer[3].Normal = { 0.0f,1.0f,0.0f };
	buffer[0].TexCoord = { 0.0f,0.0f };
	buffer[1].TexCoord = { 0.0f,10.0f };
	buffer[2].TexCoord = { 10.0f,0.0f };
	buffer[3].TexCoord = { 10.0f,10.0f };

	m_VertexBuffer->Unmap(0, nullptr);

	// テクスチャ読み込み
	m_Texture.Load("asset/field004.tga");
}

void CField::Update()
{
}

void CField::Draw(ID3D12GraphicsCommandList* CommandList)
{
	HRESULT hr;

	//マトリクス設定
	//XMMATRIX view = XMMatrixLookAtLH({ 0.0,2.0,-5.0 }, { 0.0,0.0,0.0 }, { 0.0,1.0,0.0 });
	//XMMATRIX projection = XMMatrixPerspectiveFovLH(1.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 1.0f, 20.0f);
	XMMATRIX view = CRenderer::GetInstance()->GetCamera3D()->GetViewMatrix();
	XMMATRIX projection = CRenderer::GetInstance()->GetCamera3D()->GetProjectionMatrix();

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
	vertexView.SizeInBytes = sizeof(Vertex3D) * 4;
	CommandList->IASetVertexBuffers(0, 1, &vertexView);

	//テクスチャ設定
	ID3D12DescriptorHeap* dh[] = { *m_Texture.GetDescriptorHeap().GetAddressOf() };
	CommandList->SetDescriptorHeaps(_countof(dh), dh);
	CommandList->SetGraphicsRootDescriptorTable(1, m_Texture.GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

	//トボロジ設定
	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//描画
	CommandList->DrawInstanced(4, 1, 0, 0);
}