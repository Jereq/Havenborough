#include "Camera.h"

Camera::Camera()
	: m_Forward(0.f, 0.f, 1.f),
	m_Up(0.f, 1.f, 0.f),
	m_ViewportSize(1.f, 1.f),
	m_FOV_Y(90.f)
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

Vector3 Camera::getRight() const
{
	using namespace DirectX;

	XMVECTOR forward = XMLoadFloat3(&m_Forward);
	XMVECTOR up = XMLoadFloat3(&m_Up);
	XMVECTOR right = XMVector3Cross(up, forward);

	Vector3 vRight;
	XMStoreFloat3(&vRight, right);
	return vRight;
}

const Vector2& Camera::getViewportSize() const
{
	return m_ViewportSize;
}

float Camera::getFOV_Y() const
{
	return m_FOV_Y;
}

void Camera::setViewport(const Vector2& p_Size, float p_FOV_Y)
{
	m_ViewportSize = p_Size;
	m_FOV_Y = p_FOV_Y;
}

void Camera::setPosition(const Vector3 &p_Position)
{
    m_Position = p_Position;
}

void Camera::translate(const Vector3& p_Offset)
{
	m_Position = m_Position + p_Offset;
}

void Camera::rotate(float p_Yaw, float p_Pitch, float p_Roll)
{
	using namespace DirectX;

	XMVECTOR forward = XMLoadFloat3(&m_Forward);
	XMVECTOR up = XMLoadFloat3(&m_Up);

	XMMATRIX yaw = XMMatrixRotationAxis(XMVectorSet(0.f, 1.f, 0.f, 0.f), p_Yaw);
	XMMATRIX pitch = XMMatrixRotationAxis(XMVector3Cross(up, forward), p_Pitch);
	XMMATRIX roll = XMMatrixRotationAxis(forward, p_Roll);
	XMMATRIX rotation = pitch * roll * yaw;

	forward = XMVector3Normalize(XMVector3Transform(forward, rotation));
	up = XMVector3Normalize(XMVector3Transform(up, rotation));

	XMStoreFloat3(&m_Forward, forward);
	XMStoreFloat3(&m_Up, up);
}
