#pragma once

#include <qevent.h>
#include <qmainwindow.h>
#include <qstatusbar.h>
#include <qvector3d.h>
#include <qwidget.h>

class DXWidget : public QWidget
{
	Q_OBJECT

protected:
	bool m_StandBy;
	double m_LastRendered;
	QPointF m_ClickPos;

public:
	DXWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0)
		: QWidget(parent, flags)
	{
		setAttribute(Qt::WA_PaintOnScreen);
		setAttribute(Qt::WA_NoSystemBackground);

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
		switch (e->key())
		{
		case Qt::Key::Key_Escape:
		default:
			QWidget::keyPressEvent(e);
		}
	}

	static bool isCameraOperation(QMouseEvent* e)
	{
		Q_UNUSED(e);
		return true;
	}

	void mousePressEvent(QMouseEvent* e) override
	{
		m_ClickPos = e->localPos();

		if (isCameraOperation(e))
		{
			//m_Camera->backup();

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
				//QPointF delta = (e->localPos() - m_ClickPos) / (float)height() * 180.f;
				//m_Camera->recover();
				//rotateCamera(delta.x(), delta.y(), 0.f);
				update();
			}
			else if ((e->buttons() & Qt::RightButton) && !(e->buttons() & Qt::LeftButton))
			{
				//QPointF delta = (e->localPos() - m_ClickPos) / (float)height() * m_Camera->getCenterOfInterest();
				//m_Camera->recover();
				//moveCamera(0, 0, delta.y());
				update();
			}
			else if (e->buttons() & Qt::MiddleButton)
			{
				//QPointF delta = (e->localPos() - m_ClickPos) / (float)height() * m_Camera->getCenterOfInterest();
				//m_Camera->recover();
				//moveCamera(-delta.x(), delta.y(), 0.f);
				update();
			}
		}

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
