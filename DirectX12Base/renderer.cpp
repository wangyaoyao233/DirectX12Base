#include "main.h"
#include "renderer.h"

CRenderer* CRenderer::m_Instance = nullptr;

CRenderer::CRenderer()
{
	assert(!m_Instance);
	m_Instance = this;

	m_WindowHandle = GetWindow();
	m_WindowWidth = SCREEN_WIDTH;
	m_WindowHeight = SCREEN_HEIGHT;
	m_Frame = 0;
	m_RTIndex = 0;

	m_Viewport.TopLeftX = 0.f;
	m_Viewport.TopLeftY = 0.f;
	m_Viewport.Width = (FLOAT)m_WindowWidth;
	m_Viewport.Height = (FLOAT)m_WindowHeight;
	m_Viewport.MinDepth = 0.f;
	m_Viewport.MaxDepth = 1.f;

	m_ScissorRect.top = 0;
	m_ScissorRect.left = 0;
	m_ScissorRect.right = m_WindowWidth;
	m_ScissorRect.bottom = m_WindowHeight;

	Initialize();
	m_Polygon = std::make_unique<CPolygon>();
	m_Field = std::make_unique<CField>();
}

void CRenderer::Initialize()
{
	HRESULT hr;

	//デバイス生成
	{
		UINT flag{};
		hr = CreateDXGIFactory2(flag, IID_PPV_ARGS(m_Factory.GetAddressOf()));
		assert(SUCCEEDED(hr));

		hr = m_Factory->EnumAdapters(0, (IDXGIAdapter**)m_Adapter.GetAddressOf());//ハードウェア
		//hr = m_Factory->EnumWarpAdapter(IID_PPV_ARGS(&m_Adapter));//ソフトウェア
		assert(SUCCEEDED(hr));

		hr = D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&m_Device));
		assert(SUCCEEDED(hr));
	}

	//コマンドキュー生成
	{
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};

		commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		commandQueueDesc.Priority = 0;
		commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDesc.NodeMask = 0;

		hr = m_Device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_CommandQueue.GetAddressOf()));
		assert(SUCCEEDED(hr));

		m_FenceEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		assert(m_FenceEvent);

		hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_Fence.GetAddressOf()));
		assert(SUCCEEDED(hr));
	}

	//スワップチェーン生成
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		ComPtr<IDXGISwapChain> swapChain{};

		swapChainDesc.BufferDesc.Width = m_WindowWidth;
		swapChainDesc.BufferDesc.Height = m_WindowHeight;
		swapChainDesc.OutputWindow = m_WindowHandle;
		swapChainDesc.Windowed = TRUE;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;

		hr = m_Factory->CreateSwapChain(m_CommandQueue.Get(), &swapChainDesc, swapChain.GetAddressOf());
		assert(SUCCEEDED(hr));

		hr = swapChain.As(&m_SwapChain);
		assert(SUCCEEDED(hr));

		m_RTIndex = m_SwapChain->GetCurrentBackBufferIndex();
	}

	//レンダーターゲット生成
	{
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};

		heapDesc.NumDescriptors = 2;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;
		hr = m_Device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_DescriptorHeap.GetAddressOf()));
		assert(SUCCEEDED(hr));

		UINT size = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		for (UINT i = 0; i < 2; ++i)
		{
			hr = m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTarget[i]));
			assert(SUCCEEDED(hr));

			m_RTHandle[i] = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			m_RTHandle[i].ptr += size * i;
			m_Device->CreateRenderTargetView(m_RenderTarget[i].Get(), nullptr, m_RTHandle[i]);
		}
	}

	//デプス・ステンシルバッファ生成
	{
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
		descriptorHeapDesc.NumDescriptors = 1;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NodeMask = 0;
		hr = m_Device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&m_DHDS));
		assert(SUCCEEDED(hr));

		D3D12_HEAP_PROPERTIES heapProperties{};
		D3D12_RESOURCE_DESC resourceDesc{};
		D3D12_CLEAR_VALUE clearValue{};

		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 0;
		heapProperties.VisibleNodeMask = 0;

		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resourceDesc.Width = m_WindowWidth;
		resourceDesc.Height = m_WindowHeight;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 0;
		resourceDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		hr = m_Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_DepthBuffer));
		assert(SUCCEEDED(hr));

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_DSHandle = m_DHDS->GetCPUDescriptorHandleForHeapStart();

		m_Device->CreateDepthStencilView(m_DepthBuffer.Get(), &dsvDesc, m_DSHandle);
	}

	//コマンドリスト生成
	{
		hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
		assert(SUCCEEDED(hr));

		hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_GraphicsCommandList));
		assert(SUCCEEDED(hr));
	}

	////////////////////////////////////
	//ルートシグネチャ生成
	{
		D3D12_ROOT_PARAMETER rootParameters[2]{};

		rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParameters[0].Descriptor.ShaderRegister = 0;
		rootParameters[0].Descriptor.RegisterSpace = 0;

		D3D12_DESCRIPTOR_RANGE range[1]{};
		range[0].NumDescriptors = 1;
		range[0].BaseShaderRegister = 0;
		range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParameters[1].DescriptorTable.pDescriptorRanges = &range[0];

		//サンプラー
		D3D12_STATIC_SAMPLER_DESC samplerDesc{};
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 16;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.ShaderRegister = 0;
		samplerDesc.RegisterSpace = 0;
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		D3D12_ROOT_SIGNATURE_DESC rootSignature_desc{};
		rootSignature_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSignature_desc.NumParameters = _countof(rootParameters);
		rootSignature_desc.pParameters = rootParameters;
		rootSignature_desc.NumStaticSamplers = 1;
		rootSignature_desc.pStaticSamplers = &samplerDesc;

		ComPtr<ID3DBlob> blob{};
		hr = D3D12SerializeRootSignature(&rootSignature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, nullptr);
		assert(SUCCEEDED(hr));

		hr = m_Device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
		assert(SUCCEEDED(hr));
	}

	//パイプラインステート生成
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};

		std::vector<char> vertexShader;
		std::vector<char> pixelShader;

		//頂点シェーダー読み込み
		{
			std::ifstream file("vertexShader.cso", std::ios_base::in | std::ios_base::binary);
			assert(file);

			file.seekg(0, std::ios_base::end);
			int filesize = (int)file.tellg();
			file.seekg(0, std::ios_base::beg);

			vertexShader.resize(filesize);
			file.read(&vertexShader[0], filesize);

			file.close();

			pipelineStateDesc.VS.pShaderBytecode = &vertexShader[0];
			pipelineStateDesc.VS.BytecodeLength = filesize;
		}
		//ピクセルシェーバー読み込み
		{
			std::ifstream file("pixelShader.cso", std::ios_base::in | std::ios_base::binary);
			assert(file);

			file.seekg(0, std::ios_base::end);
			int filesize = (int)file.tellg();
			file.seekg(0, std::ios_base::beg);

			pixelShader.resize(filesize);
			file.read(&pixelShader[0], filesize);

			file.close();

			pipelineStateDesc.PS.pShaderBytecode = &pixelShader[0];
			pipelineStateDesc.PS.BytecodeLength = filesize;
		}

		//インプットレイアウト
		D3D12_INPUT_ELEMENT_DESC InputElementDesc[] =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,24,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
		};
		pipelineStateDesc.InputLayout.pInputElementDescs = InputElementDesc;
		pipelineStateDesc.InputLayout.NumElements = _countof(InputElementDesc);

		pipelineStateDesc.SampleDesc.Count = 1;
		pipelineStateDesc.SampleDesc.Quality = 0;
		pipelineStateDesc.SampleMask = UINT_MAX;

		pipelineStateDesc.NumRenderTargets = 1;
		pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;

		pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateDesc.pRootSignature = m_RootSignature.Get();

		//ラスタライザステート
		pipelineStateDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		pipelineStateDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		pipelineStateDesc.RasterizerState.FrontCounterClockwise = FALSE;
		pipelineStateDesc.RasterizerState.DepthBias = 0;
		pipelineStateDesc.RasterizerState.DepthBiasClamp = 0;
		pipelineStateDesc.RasterizerState.SlopeScaledDepthBias = 0;
		pipelineStateDesc.RasterizerState.DepthClipEnable = TRUE;
		pipelineStateDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
		pipelineStateDesc.RasterizerState.AntialiasedLineEnable = FALSE;
		pipelineStateDesc.RasterizerState.MultisampleEnable = FALSE;

		//ブレンドステート
		for (int i = 0; i < _countof(pipelineStateDesc.BlendState.RenderTarget); ++i)
		{
			pipelineStateDesc.BlendState.RenderTarget[i].BlendEnable = FALSE;
			pipelineStateDesc.BlendState.RenderTarget[i].SrcBlend = D3D12_BLEND_ONE;
			pipelineStateDesc.BlendState.RenderTarget[i].DestBlend = D3D12_BLEND_ZERO;
			pipelineStateDesc.BlendState.RenderTarget[i].BlendOp = D3D12_BLEND_OP_ADD;
			pipelineStateDesc.BlendState.RenderTarget[i].SrcBlendAlpha = D3D12_BLEND_ONE;
			pipelineStateDesc.BlendState.RenderTarget[i].DestBlendAlpha = D3D12_BLEND_ZERO;
			pipelineStateDesc.BlendState.RenderTarget[i].BlendOpAlpha = D3D12_BLEND_OP_ADD;
			pipelineStateDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			pipelineStateDesc.BlendState.RenderTarget[i].LogicOpEnable = FALSE;
			pipelineStateDesc.BlendState.RenderTarget[i].LogicOp = D3D12_LOGIC_OP_CLEAR;
		}
		pipelineStateDesc.BlendState.AlphaToCoverageEnable = FALSE;
		pipelineStateDesc.BlendState.IndependentBlendEnable = FALSE;

		//デプス、ステンシルステート
		pipelineStateDesc.DepthStencilState.DepthEnable = TRUE;
		pipelineStateDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		pipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
		pipelineStateDesc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		pipelineStateDesc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

		pipelineStateDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		pipelineStateDesc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		pipelineStateDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		pipelineStateDesc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		pipelineStateDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		pipelineStateDesc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		pipelineStateDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		pipelineStateDesc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

		hr = m_Device->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&m_PiplineState));
		assert(SUCCEEDED(hr));
	}
}

void CRenderer::Update()
{
	m_Polygon->Update();
}

void CRenderer::Draw()
{
	HRESULT hr;

	FLOAT clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	//レンダーターゲット用リソースバリア
	SetResourceBarrier(D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	//デプスバッファ・レンダーターゲットのクリア
	m_GraphicsCommandList->ClearDepthStencilView(m_DSHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
	m_GraphicsCommandList->ClearRenderTargetView(m_RTHandle[m_RTIndex], clearColor, 0, nullptr);

	//////////////
	//ルートシグネチャとパイプラインステートの設定
	m_GraphicsCommandList->SetGraphicsRootSignature(m_RootSignature.Get());
	m_GraphicsCommandList->SetPipelineState(m_PiplineState.Get());
	//////////////

	//ビューポートとシザー矩形の設定
	m_GraphicsCommandList->RSSetViewports(1, &m_Viewport);
	m_GraphicsCommandList->RSSetScissorRects(1, &m_ScissorRect);

	//レンダーターゲットの設定
	m_GraphicsCommandList->OMSetRenderTargets(1, &m_RTHandle[m_RTIndex], TRUE, &m_DSHandle);

	//オブジェクト描画
	m_Field->Draw(m_GraphicsCommandList.Get());
	m_Polygon->Draw(m_GraphicsCommandList.Get());

	//プレゼント用リソースバリア
	SetResourceBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	hr = m_GraphicsCommandList->Close();
	assert(SUCCEEDED(hr));

	//コマンドリスト実行
	ID3D12CommandList* const command_lists = m_GraphicsCommandList.Get();
	m_CommandQueue->ExecuteCommandLists(1, &command_lists);

	//実行したコマンドの終了待ち
	const UINT64 fence = m_Frame;
	hr = m_CommandQueue->Signal(m_Fence.Get(), fence);
	assert(SUCCEEDED(hr));
	m_Frame++;

	if (m_Fence->GetCompletedValue() < fence)
	{
		hr = m_Fence->SetEventOnCompletion(fence, m_FenceEvent);
		assert(SUCCEEDED(hr));

		WaitForSingleObject(m_FenceEvent, INFINITE);
	}

	hr = m_CommandAllocator->Reset();
	assert(SUCCEEDED(hr));

	hr = m_GraphicsCommandList->Reset(m_CommandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));

	hr = m_SwapChain->Present(1, 0);
	assert(SUCCEEDED(hr));

	//カレントのバックバッファのインデックスを取得する
	m_RTIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void CRenderer::SetResourceBarrier(D3D12_RESOURCE_STATES BeforeState, D3D12_RESOURCE_STATES AfterState)
{
	D3D12_RESOURCE_BARRIER resourceBarrier{};

	resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	resourceBarrier.Transition.pResource = m_RenderTarget[m_RTIndex].Get();
	resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	resourceBarrier.Transition.StateBefore = BeforeState;
	resourceBarrier.Transition.StateAfter = AfterState;

	m_GraphicsCommandList->ResourceBarrier(1, &resourceBarrier);
}