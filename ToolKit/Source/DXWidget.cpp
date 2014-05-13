#include "DXWidget.h"

#include <qevent.h>
#include <QMainWindow>
#include <QStatusBar>
#include "EventData.h"

DXWidget::DXWidget(QWidget* parent, Qt::WindowFlags flags)
	: QWidget(parent, flags),
	m_FlyControl(&m_Camera, &m_Control),
	m_EventManager(nullptr),
	m_ResourceManager(nullptr)
{
	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NoSystemBackground);

	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

void DXWidget::onFrame(float p_DeltaTime)
{
    Vector3 previousCameraPosition = m_Camera.getPosition();

	m_FlyControl.update(p_DeltaTime);
		
	Vector3 currentCameraPosition = m_Camera.getPosition();
        if(previousCameraPosition.x != currentCameraPosition.x ||
            	previousCameraPosition.y != currentCameraPosition.y ||
            	previousCameraPosition.z != currentCameraPosition.z)
            emit CameraPositionChanged(currentCameraPosition);

}

void DXWidget::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e);
	render();
}

void DXWidget::resizeEvent(QResizeEvent* p_Event)
{
	QSize newSize = size();
	if (p_Event)
	{
		newSize = p_Event->size();
		QWidget::resizeEvent(p_Event);
	}
	onResize(newSize.width(), newSize.height());
}

void DXWidget::keyPressEvent(QKeyEvent* e)
{
	if (m_Control.keyPressEvent(e))
		return;

	if (m_FlyControl.keyPressEvent(e))
		return;

	QWidget::keyPressEvent(e);
}

void DXWidget::keyReleaseEvent(QKeyEvent* e)
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

void DXWidget::mousePressEvent(QMouseEvent* e)
{
	m_PrevMousePos = e->localPos();

	if (isCameraOperation(e))
	{
		if ((e->buttons() & Qt::LeftButton) && !(e->buttons() & Qt::RightButton))
		{
			m_EventManager->triggerTriggerEvent(IEventData::Ptr(new CreateRayEventData(DirectX::XMFLOAT2(e->localPos().x(), e->localPos().y()),
																						DirectX::XMFLOAT2(width(), height()))));
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

void DXWidget::mouseMoveEvent(QMouseEvent* e)
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

void DXWidget::mouseReleaseEvent(QMouseEvent* e)
{
	setCursor(Qt::ArrowCursor);
	//showStatus("");

	QWidget::mouseReleaseEvent(e);
}

void DXWidget::wheelEvent(QWheelEvent* e)
{
	QWidget::wheelEvent(e);

	//zoomCamera(1.f - (e->delta() / WHEEL_DELTA) * 0.125f);
	update();
}

void DXWidget::CameraPositionSet(Vector3 value)
{
	if (value != m_Camera.getPosition())
	{
		m_Camera.setPosition(value);
		emit CameraPositionChanged(value);
	}
}
