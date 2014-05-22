#include "FlyControl.h"

#include <qevent.h>

FlyControl::FlyControl(Camera* p_Camera, KeyboardControl* p_Control)
	: m_Camera(p_Camera),
	  m_Control(p_Control),
	  m_MovementSpeed((float)normalMovementSpeed)
{
}

void FlyControl::update(float p_DeltaTime)
{
	Vector3 direction = m_Camera->getForward() * m_Control->m_Move
		+ m_Camera->getRight() * m_Control->m_Strafe
		+ Vector3(0.f, 1.f, 0.f) * m_Control->m_Lift;
	m_Camera->translate(direction * p_DeltaTime * m_MovementSpeed);
}

bool FlyControl::keyPressEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key::Key_Shift:
			m_MovementSpeed = slowMovementSpeed;
			return true;

		default:
			return false;
	}
}

bool FlyControl::keyReleaseEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key::Key_Shift:
			m_MovementSpeed = normalMovementSpeed;
			return true;

		default:
			return false;
	}
}
