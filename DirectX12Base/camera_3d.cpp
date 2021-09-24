#include "main.h"
#include "camera_3d.h"

CCamera3D::CCamera3D()
{
	Init();
}

void CCamera3D::Init()
{
	m_EyePos = XMFLOAT3(0.0f, 2.0f, -5.0f);
	m_FocusPos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_UpDirection = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

void CCamera3D::Update()
{
}

void CCamera3D::Draw()
{
	XMMATRIX view = XMMatrixLookAtLH(XMLoadFloat3(&m_EyePos), XMLoadFloat3(&m_FocusPos), XMLoadFloat3(&m_UpDirection));
	XMMATRIX projection = XMMatrixPerspectiveFovLH(1.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 1.0f, 20.0f);

	XMStoreFloat4x4(&m_View, view);
	XMStoreFloat4x4(&m_Projection, projection);
}