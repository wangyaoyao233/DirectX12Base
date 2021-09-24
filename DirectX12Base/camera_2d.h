#pragma once
#include "camera_base.h"

class CCamera2D : public CCameraBase
{
private:

public:
	CCamera2D();

	void Init();
	void Update();
	void Draw();
};