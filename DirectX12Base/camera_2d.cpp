#include "main.h"
#include "camera_2d.h"

CCamera2D::CCamera2D()
{
	Init();
}

void CCamera2D::Init()
{
}

void CCamera2D::Update()
{
}

void CCamera2D::Draw()
{
	XMMATRIX view = XMMatrixIdentity();
	XMMATRIX projection = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);

	XMStoreFloat4x4(&m_View, view);
	XMStoreFloat4x4(&m_Projection, projection);
}