#include "DXWidget.h"

#include <qevent.h>
#include <QMainWindow>
#include <QStatusBar>
#include "EventData.h"

#include "RotationTool.h"

DXWidget::DXWidget(QWidget* parent, Qt::WindowFlags flags)
	: QWidget(parent, flags),
	m_FlyControl(&m_Camera, &m_Control),
	m_EventManager(nullptr),
	m_ResourceManager(nullptr),
	m_RotationTool(nullptr)
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

const Camera& DXWidget::getCamera() const
{
	return m_Camera;
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
	m_ToolManager.updateScreenSize(newSize.width(), newSize.height());
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
			m_ToolManager.updateMousePos(e->localPos().x(), e->localPos().y());
			m_ToolManager.onPress();
			//showStatus(tr("Tumble Tool: LMB Drag: Use LMB or MMB to tumble"));
			setCursor(Qt::OpenHandCursor);
		}
		else if ((e->buttons() & Qt::RightButton) && !(e->buttons() & Qt::LeftButton))
		{
			//showStatus(tr("Dolly Tool: RMB Drag: Use mouse to dolly"));
			//setCursor(Qt::BlankCursor);

			m_MouseStartPos = e->localPos();
			m_MouseDir = QPointF(0, 0);
		
			std::shared_ptr<MouseEventDataPie> pie(new MouseEventDataPie(Vector2(m_MouseStartPos.x() - width()*0.5f, -m_MouseStartPos.y() + height()*0.5f), true));
			m_EventManager->queueEvent(pie);
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
			if (e->modifiers() & Qt::Modifier::CTRL)
			{
				m_Camera.rotate(delta.x(), delta.y(), 0.f);
			}
			else
			{
				m_ToolManager.onMove();
				m_RotationTool->mouseMovement(delta);
			}

			update();
		}
		else if ((e->buttons() & Qt::RightButton) && !(e->buttons() & Qt::LeftButton))
		{
			//QPointF delta = (e->localPos() - m_ClickPos) / (float)height() * m_Camera->getCenterOfInterest();
			//moveCamera(0, 0, delta.y());

			QPointF mouseDir = e->localPos() - m_PrevMousePos;

			qreal dotMouseDir = QPointF::dotProduct(mouseDir, mouseDir);
			dotMouseDir = sqrtf(dotMouseDir);
			mouseDir = mouseDir / dotMouseDir;

			m_MouseDir = m_MouseDirPrev * 0.95f + mouseDir * 0.05f;

			float a = atan2f(-m_MouseDir.x(), m_MouseDir.y());

			a += DirectX::XM_PI;
			a += (2 * DirectX::XM_PI) / (m_PowerPie.nrOfElements * 2.f);
			if(a > 2 * DirectX::XM_PI)
				a -= 2*DirectX::XM_PI;

			a /= (2*DirectX::XM_PI);
			a *= m_PowerPie.nrOfElements;

			a = floorf(a);
			m_PieAngleIndex = a;
			if(m_PieAngleIndex >= m_PowerPie.nrOfElements)
				m_PieAngleIndex = -1;

			std::shared_ptr<PowerPieSelectEventData> pie(new PowerPieSelectEventData(a));
			m_EventManager->queueEvent(pie);

			((QMainWindow*)window())->statusBar()->showMessage("Angle: " + QString::number(a));

			m_MouseDirPrev = m_MouseDir;

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

	//((QMainWindow*)window())->statusBar()->showMessage("PosX: " + QString::number(e->localPos().x()) + " PosY: " + QString::number(e->localPos().y()));

	QWidget::mouseMoveEvent(e);
}

void DXWidget::mouseReleaseEvent(QMouseEvent* e)
{
	m_RotationTool->mouseReleased();

	setCursor(Qt::ArrowCursor);
	
	//showStatus("");
	if ((e->button() & Qt::RightButton) && !(e->button() & Qt::LeftButton))
	{
		m_ToolManager.changeTool(m_PieAngleIndex);
		//showStatus(tr("Tumble Tool: LMB Drag: Use LMB or MMB to tumble"));
	}


	std::shared_ptr<MouseEventDataPie> pie(new MouseEventDataPie(Vector2(0.f, 0.f), false));
	m_EventManager->queueEvent(pie);

	int x = m_MouseStartPos.x();
	int y = m_MouseStartPos.y();
	QPoint temp = mapToGlobal(QPoint(x, y));
	//QCursor::setPos(temp);
	
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
