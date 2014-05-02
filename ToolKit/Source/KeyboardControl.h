#pragma once

#include <qevent.h>

class KeyboardControl
{
public:
	float m_Strafe;
	float m_Move;
	float m_Lift;

private:
	float m_MoveLeft, m_MoveRight;
	float m_MoveForward, m_MoveBackward;
	float m_MoveUp, m_MoveDown;

public:
	KeyboardControl();

	bool keyPressEvent(QKeyEvent* e);
	bool keyReleaseEvent(QKeyEvent* e);

	void startMoveForward()
	{
		m_MoveForward = 1.f;
		m_Move = m_MoveForward - m_MoveBackward;
	}

private:
	void startMoveBackward()
	{
		m_MoveBackward = 1.f;
		m_Move = m_MoveForward - m_MoveBackward;
	}

	void startMoveLeft()
	{
		m_MoveLeft = 1.f;
		m_Strafe = m_MoveRight - m_MoveLeft;
	}

	void startMoveRight()
	{
		m_MoveRight = 1.f;
		m_Strafe = m_MoveRight - m_MoveLeft;
	}

	void startMoveUp()
	{
		m_MoveUp = 1.f;
		m_Lift = m_MoveUp - m_MoveDown;
	}

	void startMoveDown()
	{
		m_MoveDown = 1.f;
		m_Lift = m_MoveUp - m_MoveDown;
	}

	void stopMoveForward()
	{
		m_MoveForward = 0.f;
		m_Move = m_MoveForward - m_MoveBackward;
	}

	void stopMoveBackward()
	{
		m_MoveBackward = 0.f;
		m_Move = m_MoveForward - m_MoveBackward;
	}

	void stopMoveLeft()
	{
		m_MoveLeft = 0.f;
		m_Strafe = m_MoveRight - m_MoveLeft;
	}

	void stopMoveRight()
	{
		m_MoveRight = 0.f;
		m_Strafe = m_MoveRight - m_MoveLeft;
	}

	void stopMoveUp()
	{
		m_MoveUp = 0.f;
		m_Lift = m_MoveUp - m_MoveDown;
	}

	void stopMoveDown()
	{
		m_MoveDown = 0.f;
		m_Lift = m_MoveUp - m_MoveDown;
	}
};
