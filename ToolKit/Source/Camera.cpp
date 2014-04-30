#include "Camera.h"

Camera::Camera()
	: m_Forward(0.f, 0.f, 1.f),
	m_Up(0.f, 1.f, 0.f)
{
}

const Vector3& Camera::getPosition() const
{
	return m_Position;
}

const Vector3& Camera::getForward() const
{
	return m_Forward;
}

const Vector3& Camera::getUp() const
{
	return m_Up;
}

void Camera::backup()
{
	m_BackupPosition = m_Position;
	m_BackupForward = m_Forward;
	m_BackupUp = m_Up;
}

void Camera::recover()
{
	m_Position = m_BackupPosition;
	m_Forward = m_BackupForward;
	m_Up = m_BackupUp;
}

void Camera::rotate(float p_Yaw, float p_Pitch, float p_Roll)
{
	using namespace DirectX;

	XMVECTOR position = XMVectorSet(m_Position.x, m_Position.y, m_Position.z, 1.f);
	XMVECTOR forward = XMLoadFloat3(&m_Forward);
	XMVECTOR up = XMLoadFloat3(&m_Up);

	XMMATRIX yaw = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), p_Yaw);
	XMMATRIX pitch = XMMatrixRotationAxis(XMVector3Cross(up, forward), p_Pitch);
	XMMATRIX roll = XMMatrixRotationAxis(forward, p_Roll);
	XMMATRIX rotation = pitch * roll * yaw;

	position = XMVector4Transform(position, rotation);
	forward = XMVector3Normalize(XMVector3Transform(forward, rotation));
	up = XMVector3Normalize(XMVector3Transform(up, rotation));

	XMStoreFloat3(&m_Position, position);
	XMStoreFloat3(&m_Forward, forward);
	XMStoreFloat3(&m_Up, up);
}
