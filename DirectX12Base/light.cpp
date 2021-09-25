#include "main.h"
#include "renderer.h"
#include "light.h"

CLight::CLight()
{
	Initialize();
}

void CLight::Initialize()
{
	m_Direction = { 1.0f,-1.0f,1.0f, 0.0f };
}

void CLight::Update()
{
}