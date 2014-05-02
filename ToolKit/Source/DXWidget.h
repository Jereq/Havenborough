#pragma once

#include <qevent.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qvector3d.h>
#include <qwidget.h>

#include "Camera.h"
#include "FlyControl.h"
#include "KeyboardControl.h"

class DXWidget : public QWidget
{
	Q_OBJECT

protected:
	Camera m_Camera;
	KeyboardControl m_Control;
	FlyControl m_FlyControl;
	bool m_StandBy;
	double m_LastRendered;
	QPointF m_PrevMousePos;

public:
	DXWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0)
		: QWidget(parent, flags),
		m_FlyControl(&m_Camera, &m_Control)
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
		m_FlyControl.update(p_DeltaTime);
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

	void keyPressEvent(QKeyEvent* e) override
	{
		if (m_Control.keyPressEvent(e))
			return;

		if (m_FlyControl.keyPressEvent(e))
			return;

		QWidget::keyPressEvent(e);
	}

	void keyReleaseEvent(QKeyEvent* e) override
	{
		if (m_Control.keyReleaseEvent(e))
			return;

		if (m_FlyControl.keyReleaseEvent(e))
			return;

		QWidget::keyReleaseEvent(e);
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
				setCursor(Qt::SizeVerCursor);
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
