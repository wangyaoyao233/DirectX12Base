#pragma once

class CCameraBase
{
protected:
	XMFLOAT4X4 m_View;
	XMFLOAT4X4 m_Projection;

public:
	CCameraBase();

	void Init();
	void Update();
	void Draw();

	XMMATRIX GetViewMatrix()
	{
		return XMLoadFloat4x4(&m_View);
	}
	XMMATRIX GetProjectionMatrix()
	{
		return XMLoadFloat4x4(&m_Projection);
	}
};