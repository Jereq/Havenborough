#pragma once

#include <qevent.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qvector3d.h>
#include <qwidget.h>

#include "Camera.h"

class DXWidget : public QWidget
{
	Q_OBJECT

protected:
	Camera m_Camera;
	bool m_StandBy;
	double m_LastRendered;
	QPointF m_PrevMousePos;
	float m_MoveLeft, m_MoveRight, m_Strafe;
	float m_MoveForward, m_MoveBackward, m_Move;
	float m_MoveUp, m_MoveDown, m_Lift;

	static const int normalMovementSpeed = 1000;
	static const int slowMovementSpeed = 100;
	float m_MovementSpeed;

	QPointF m_MouseStartPos;
	QPointF m_MouseDir;

public:
	DXWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0)
		: QWidget(parent, flags),
		m_MoveLeft(0.f),
		m_MoveRight(0.f),
		m_Strafe(0.f),
		m_MoveForward(0.f),
		m_MoveBackward(0.f),
		m_Move(0.f),
		m_MoveUp(0.f),
		m_MoveDown(0.f),
		m_Lift(0.f),
		m_MovementSpeed(normalMovementSpeed)
	{
		setAttribute(Qt::WA_PaintOnScreen);
		setAttribute(Qt::WA_NoSystemBackground);

		setFocusPolicy(Qt::FocusPolicy::StrongFocus);

		m_StandBy = false;
		m_LastRendered = 0;

	}

	virtual ~DXWidget()
	{
	}

	virtual void initialize() = 0;
	virtual void uninitialize() = 0;

	virtual void restoreDeviceObjects() = 0;
	virtual void invalidateDeviceObjects() = 0;

	virtual void render() {}
	virtual void present() {}

	void updateStep(float p_DeltaTime)
	{
		Vector3 direction = m_Camera.getForward() * m_Move
			+ m_Camera.getRight() * m_Strafe
			+ Vector3(0.f, 1.f, 0.f) * m_Lift;
		m_Camera.translate(direction * p_DeltaTime * m_MovementSpeed);
	}

protected:
	QPaintEngine* paintEngine() const { return nullptr; }
	virtual void onResize(unsigned int, unsigned int) = 0;

	void paintEvent(QPaintEvent* e) override
	{
		Q_UNUSED(e);
		render();
	}

	void resizeEvent(QResizeEvent* p_Event) override
	{
		QSize newSize = size();
		if (p_Event)
		{
			newSize = p_Event->size();
			QWidget::resizeEvent(p_Event);
		}
		onResize(newSize.width(), newSize.height());
	}

	void startMoveForward()
	{
		m_MoveForward = 1.f;
		m_Move = m_MoveForward - m_MoveBackward;
	}

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

	void keyPressEvent(QKeyEvent* e) override
	{
		switch (e->key())
		{
		case Qt::Key::Key_W:
			startMoveForward();
			break;

		case Qt::Key::Key_S:
			startMoveBackward();
			break;

		case Qt::Key::Key_A:
			startMoveLeft();
			break;

		case Qt::Key::Key_D:
			startMoveRight();
			break;

		case Qt::Key::Key_Space:
			startMoveUp();
			break;

		case Qt::Key::Key_Shift:
			startMoveDown();
			break;

		case Qt::Key::Key_Control:
			m_MovementSpeed = slowMovementSpeed;
			break;

		default:
			QWidget::keyPressEvent(e);
		}
	}

	void keyReleaseEvent(QKeyEvent* e) override
	{
		switch (e->key())
		{
		case Qt::Key::Key_W:
			stopMoveForward();
			break;

		case Qt::Key::Key_S:
			stopMoveBackward();
			break;

		case Qt::Key::Key_A:
			stopMoveLeft();
			break;

		case Qt::Key::Key_D:
			stopMoveRight();
			break;

		case Qt::Key::Key_Space:
			stopMoveUp();
			break;

		case Qt::Key::Key_Shift:
			stopMoveDown();
			break;

		case Qt::Key::Key_Control:
			m_MovementSpeed = normalMovementSpeed;
			break;

		default:
			QWidget::keyReleaseEvent(e);
		}
	}

	static bool isCameraOperation(QMouseEvent* e)
	{
		Q_UNUSED(e);
		return true;
	}

	void mousePressEvent(QMouseEvent* e) override
	{
		m_PrevMousePos = e->localPos();

		if (isCameraOperation(e))
		{
			if ((e->buttons() & Qt::LeftButton) && !(e->buttons() & Qt::RightButton))
			{
				//showStatus(tr("Tumble Tool: LMB Drag: Use LMB or MMB to tumble"));
				setCursor(Qt::OpenHandCursor);
			}
			else if ((e->buttons() & Qt::RightButton) && !(e->buttons() & Qt::LeftButton))
			{
				//showStatus(tr("Dolly Tool: RMB Drag: Use mouse to dolly"));
				setCursor(Qt::CrossCursor);

				m_MouseStartPos = e->localPos();
				m_MouseDir = QPointF(0, 0);

			}
			else if (e->buttons() & Qt::MiddleButton)
			{
				//showStatus(tr("Track Tool: MMB Drag: Use LMB or MMB to track"));
				setCursor(Qt::SizeAllCursor);
			}
		}

		QWidget::mousePressEvent(e);
	}

	void mouseMoveEvent(QMouseEvent* e) override
	{
		if (isCameraOperation(e) && height() > 0)
		{
			if ((e->buttons() & Qt::LeftButton) && !(e->buttons() & Qt::RightButton))
			{
				QPointF delta = (e->localPos() - m_PrevMousePos) / (float)height() * DirectX::XM_PI;
				m_Camera.rotate(delta.x(), delta.y(), 0.f);
				update();
			}
			else if ((e->buttons() & Qt::RightButton) && !(e->buttons() & Qt::LeftButton))
			{
				//QPointF delta = (e->localPos() - m_ClickPos) / (float)height() * m_Camera->getCenterOfInterest();
				//moveCamera(0, 0, delta.y());

				m_MouseDir = e->localPos() - m_MouseStartPos;
				qreal dotMouseDir = QPointF::dotProduct(m_MouseDir, m_MouseDir);
				m_MouseDir = m_MouseDir / dotMouseDir;


				update();
			}
			else if (e->buttons() & Qt::MiddleButton)
			{
				//QPointF delta = (e->localPos() - m_ClickPos) / (float)height() * m_Camera->getCenterOfInterest();
				//moveCamera(-delta.x(), delta.y(), 0.f);
				update();
			}
		}

		m_PrevMousePos = e->localPos();

		QWidget::mouseMoveEvent(e);
	}

	void mouseReleaseEvent(QMouseEvent* e) override
	{
		setCursor(Qt::ArrowCursor);
		//showStatus("");

		QWidget::mouseReleaseEvent(e);
	}

	void wheelEvent(QWheelEvent* e) override
	{
		QWidget::wheelEvent(e);

		//zoomCamera(1.f - (e->delta() / WHEEL_DELTA) * 0.125f);
		update();
	}
};
