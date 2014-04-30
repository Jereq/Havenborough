#pragma once

#include <Utilities/XMFloatUtil.h>

class Camera
{
	Vector3 m_Position;
	Vector3 m_Forward;
	Vector3 m_Up;

	Vector3 m_BackupPosition;
	Vector3 m_BackupForward;
	Vector3 m_BackupUp;

public:
	Camera();

	const Vector3& getPosition() const;
	const Vector3& getForward() const;
	const Vector3& getUp() const;

	void backup();
	void recover();

	void rotate(float p_Yaw, float p_Pitch, float p_Roll);
};
