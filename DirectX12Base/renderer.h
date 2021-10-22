#pragma once

using namespace DirectX;
using namespace Microsoft::WRL;

#include "polygon.h"
#include "polygon_deferred.h"
#include "field.h"
#include "cube.h"
#include "camera_2d.h"
#include "camera_3d.h"
#include "light.h"

struct Vertex3D
{
	XMFLOAT3 Position;
	XMFLOAT3 Normal;
	XMFLOAT2 TexCoord;
};

struct Constant
{
	XMFLOAT4X4 WVP;
	XMFLOAT4X4 World;
	//XMFLOAT4 LightDirection;
	//XMFLOAT4 CameraPostion;
};

class CRenderer
{
private:
	static const int NUM_GBUFFER = 2;

	static CRenderer* m_Instance;

	HWND								m_WindowHandle;
	int									m_WindowWidth;
	int									m_WindowHeight;

	UINT64								m_Frame;
	UINT								m_RTIndex;

	ComPtr<IDXGIFactory4>				m_Factory;
	ComPtr<IDXGIAdapter3>				m_Adapter;
	ComPtr<ID3D12Device>				m_Device;
	ComPtr<ID3D12CommandQueue>			m_CommandQueue;
	HANDLE								m_FenceEvent;
	ComPtr<ID3D12Fence>					m_Fence;
	ComPtr<IDXGISwapChain3>				m_SwapChain;
	ComPtr<ID3D12GraphicsCommandList>	m_GraphicsCommandList;
	ComPtr<ID3D12CommandAllocator>		m_CommandAllocator;
	ComPtr<ID3D12Resource>				m_RenderTarget[2];
	ComPtr<ID3D12DescriptorHeap>		m_DescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_RTHandle[2];
	ComPtr<ID3D12Resource>				m_DepthBuffer;
	ComPtr<ID3D12DescriptorHeap>		m_DHDS;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_DSHandle;

	//ComPtr<ID3D12PipelineState>			m_PiplineState;
	ComPtr<ID3D12PipelineState>			m_PipelineStateGeometry;
	ComPtr<ID3D12PipelineState>			m_PipelineStateLight;

	ComPtr<ID3D12RootSignature>			m_RootSignature;

	D3D12_RECT							m_ScissorRect;
	D3D12_VIEWPORT						m_Viewport;

	//Deferred Render
	ComPtr<ID3D12Resource>				m_NormalResource;
	ComPtr<ID3D12Resource>				m_DiffuseResource;
	ComPtr<ID3D12Resource>				m_PositionResource;
	ComPtr<ID3D12Resource>				m_DepthResource;
	D3D12_CPU_DESCRIPTOR_HANDLE			m_RTHandleGeometry[NUM_GBUFFER];
	ComPtr<ID3D12DescriptorHeap>		m_RTVDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap>		m_SRVDescriptorHeap;

	std::unique_ptr<CPolygon>			m_Polygon;
	std::unique_ptr<CField>				m_Field;
	std::unique_ptr<CCube>				m_Cube;

	std::unique_ptr<CCamera2D>			m_Camera2D;
	std::unique_ptr<CCamera3D>			m_Camera3D;

	//std::unique_ptr<CLight>				m_Light;

	std::unique_ptr<CPolygonDeferred>	m_PolygonDeferred;

public:

	CRenderer();

	void Initialize();
	void Update();
	void Draw();
	void SetResourceBarrier(ID3D12Resource* Resource, D3D12_RESOURCE_STATES BeforeState, D3D12_RESOURCE_STATES AfterState);

	static CRenderer* GetInstance() { return m_Instance; }
	ComPtr<ID3D12Device> GetDevice() { return m_Device; }

	CCamera2D* GetCamera2D() { return m_Camera2D.get(); }
	CCamera3D* GetCamera3D() { return m_Camera3D.get(); }
	//CLight* GetLight() { return m_Light.get(); }
};
