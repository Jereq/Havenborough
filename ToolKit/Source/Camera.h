#pragma once

#include <Utilities/XMFloatUtil.h>

class Camera
{
	Vector3 m_Position;
	Vector3 m_Forward;
	Vector3 m_Up;
	Vector2 m_ViewportSize;
	float m_FOV_Y;

public:
	Camera();

	const Vector3& getPosition() const;
	const Vector3& getForward() const;
	const Vector3& getUp() const;
	Vector3 getRight() const;

	const Vector2& getViewportSize() const;
	float getFOV_Y() const;

	void setViewport(const Vector2& p_Size, float p_FOV_Y);
    void setPosition(const Vector3& p_Position);

	void translate(const Vector3& p_Offset);
	void rotate(float p_Yaw, float p_Pitch, float p_Roll);
};
