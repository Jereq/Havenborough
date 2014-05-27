#pragma once
#include "Utilities\XMFloatUtil.h"
#include <d3d11.h>

struct BillboardRenderable
{
	DirectX::XMFLOAT3 m_Position;
	DirectX::XMFLOAT2 m_Size;
	float m_Scale;
	float m_Rotation;
	ID3D11ShaderResourceView *m_Texture;

	BillboardRenderable(DirectX::XMFLOAT3 p_Position, DirectX::XMFLOAT2 p_Size, float p_Scale,
		float p_Rotation, ID3D11ShaderResourceView *p_Texture)
	{
		m_Position = p_Position;
		m_Size = p_Size;
		m_Scale = p_Scale;
		m_Rotation = p_Rotation;
		m_Texture = p_Texture;
	}
};