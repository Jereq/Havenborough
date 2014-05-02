#pragma once

#include <Utilities/XMFloatUtil.h>

class Camera
{
	Vector3 m_Position;
	Vector3 m_Forward;
	Vector3 m_Up;

public:
	Camera();

	const Vector3& getPosition() const;
	const Vector3& getForward() const;
	const Vector3& getUp() const;
	Vector3 getRight() const;

	void translate(const Vector3& p_Offset);
	void rotate(float p_Yaw, float p_Pitch, float p_Roll);
};
