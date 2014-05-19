#include "CameraInterpolation.h"	

CameraInterpolation::CameraInterpolation() : 
	m_Active(false), m_Timer(0.f), m_CurrentTimer(0.f), m_StartPos(Vector3(0,0,0)), m_EndPos(Vector3(0,0,0))
{
}

void CameraInterpolation::update(float p_dt)
{
	if(m_Active)
	{
		using namespace DirectX;

		XMVECTOR start = XMLoadFloat3(&m_StartPos);
		XMVECTOR path = XMLoadFloat3(&m_EndPos) - start;
		Vector3 currPosition;

		XMStoreFloat3(&currPosition, start + path * (m_CurrentTimer / m_Timer));

		emit setCameraPositionSignal(currPosition);

		m_CurrentTimer += p_dt;
		if(m_CurrentTimer > m_Timer)
			m_Active = false;
	}
}

void CameraInterpolation::createPath(Vector3 p_StartPos, Vector3 p_EndPos, float p_Time)
{
	if(!m_Active)
	{
		m_Active = true;
		m_StartPos = p_StartPos;
		m_EndPos = p_EndPos;
		m_Timer = p_Time;
		m_CurrentTimer = 0.f;
	}
}
