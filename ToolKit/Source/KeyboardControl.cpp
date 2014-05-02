#include "KeyboardControl.h"

KeyboardControl::KeyboardControl()
	:	m_MoveLeft(0.f),
		m_MoveRight(0.f),
		m_Strafe(0.f),
		m_MoveForward(0.f),
		m_MoveBackward(0.f),
		m_Move(0.f),
		m_MoveUp(0.f),
		m_MoveDown(0.f),
		m_Lift(0.f)
{
}

bool KeyboardControl::keyPressEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key::Key_W:
			startMoveForward();
			return true;

		case Qt::Key::Key_S:
			startMoveBackward();
			return true;

		case Qt::Key::Key_A:
			startMoveLeft();
			return true;

		case Qt::Key::Key_D:
			startMoveRight();
			return true;

		case Qt::Key::Key_Space:
			startMoveUp();
			return true;

		case Qt::Key::Key_Shift:
			startMoveDown();
			return true;

		default:
			return false;
	}
}

bool KeyboardControl::keyReleaseEvent(QKeyEvent* e)
{
	switch (e->key())
	{
		case Qt::Key::Key_W:
			stopMoveForward();
			return true;

		case Qt::Key::Key_S:
			stopMoveBackward();
			return true;

		case Qt::Key::Key_A:
			stopMoveLeft();
			return true;

		case Qt::Key::Key_D:
			stopMoveRight();
			return true;

		case Qt::Key::Key_Space:
			stopMoveUp();
			return true;

		case Qt::Key::Key_Shift:
			stopMoveDown();
			return true;

		default:
			return false;
	}
}
