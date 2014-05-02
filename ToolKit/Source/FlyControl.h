#pragma once

#include <qevent.h>

#include "Camera.h"
#include "KeyboardControl.h"

class FlyControl
{
private:
	Camera* m_Camera;
	KeyboardControl* m_Control;
	
	static const int normalMovementSpeed = 1000;
	static const int slowMovementSpeed = 100;
	float m_MovementSpeed;

public:
	FlyControl(Camera* p_Camera, KeyboardControl* p_Control);

	void update(float p_DeltaTime);
	bool keyPressEvent(QKeyEvent* e);
	bool keyReleaseEvent(QKeyEvent* e);
};

