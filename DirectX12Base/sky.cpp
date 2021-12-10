#include "sky.h"

CSky::CSky()
{
	Initialize();
}

CSky::~CSky()
{
	m_Model->Unload();
}

void CSky::Initialize()
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

	// 定数バッファの作成
	resourceDesc.Width = 256;//定数バッファは256byteアライン
	hr = device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_ConstantBuffer));
	assert(SUCCEEDED(hr));

	m_Model = std::make_unique<CModel>();
	m_Model->Load("./asset/sky.obj");

	m_Texture.Load("asset/envmap.tga");
}

void CSky::Update()
{
}

void CSky::Draw(ID3D12GraphicsCommandList* CommandList)
{
	HRESULT hr;

	//マトリクス設定
	XMMATRIX view = XMMatrixLookAtLH({ 0.0,2.0,-5.0 }, { 0.0,0.0,0.0 }, { 0.0,1.0,0.0 });
	XMMATRIX projection = XMMatrixPerspectiveFovLH(1.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 1.0f, 100.0f);
	//XMMATRIX view = CRenderer::GetInstance()->GetCamera3D()->GetViewMatrix();
	//XMMATRIX projection = CRenderer::GetInstance()->GetCamera3D()->GetProjectionMatrix();

	XMMATRIX world = XMMatrixIdentity();

	//scale
	world *= XMMatrixScaling(10, 10, 10);
	world *= XMMatrixTranslation(0.0f, 2.0f, -5.0f);

	//back

	//定数バッファ設定
	Constant* constant;
	hr = m_ConstantBuffer->Map(0, nullptr, (void**)&constant);
	assert(SUCCEEDED(hr));

	XMFLOAT4X4 matrix;
	XMStoreFloat4x4(&matrix, XMMatrixTranspose(world * view * projection));
	constant->WVP = matrix;
	XMStoreFloat4x4(&matrix, XMMatrixTranspose(world));
	constant->World = matrix;

	//constant->Param = { 0, 0, 0, 0.5 };

	m_ConstantBuffer->Unmap(0, nullptr);

	CommandList->SetGraphicsRootConstantBufferView(0, m_ConstantBuffer->GetGPUVirtualAddress());

	//テクスチャ設定
	ID3D12DescriptorHeap* dh[] = { *m_Texture.GetDescriptorHeap().GetAddressOf() };
	CommandList->SetDescriptorHeaps(_countof(dh), dh);
	CommandList->SetGraphicsRootDescriptorTable(1, m_Texture.GetDescriptorHeap()->GetGPUDescriptorHandleForHeapStart());

	m_Model->Draw(CommandList);
}
