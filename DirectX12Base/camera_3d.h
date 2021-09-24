#pragma once
#include "camera_base.h"

class CCamera3D : public CCameraBase
{
private:
	XMFLOAT3 m_EyePos;
	XMFLOAT3 m_FocusPos;
	XMFLOAT3 m_UpDirection;
public:
	CCamera3D();

	void Init();
	void Update();
	void Draw();
};