#pragma once

class CLight
{
private:
	XMFLOAT4	m_Direction;
	XMFLOAT4	m_Diffuse;
	XMFLOAT4	m_Ambient;

public:
	CLight();

	void Initialize();
	void Update();

	XMFLOAT4 GetDirection() { return m_Direction; }
	XMFLOAT4 GetDiffuse() { return m_Diffuse; }
	XMFLOAT4 GetAmbient() { return m_Ambient; }
};