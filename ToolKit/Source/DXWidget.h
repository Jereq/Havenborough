#pragma once

#include <QWidget>
#include <EventData.h>

#include "Camera.h"
#include "FlyControl.h"
#include "KeyboardControl.h"
#include "ToolManager.h"

class EventManager;
class IGraphics;
class ResourceManager;

class DXWidget : public QWidget
{
	Q_OBJECT

protected:
	struct PowerPie
	{
		Vector2 position;
		bool isActive;

		float nrOfElements;
		float angle;

		Vector4 selectedColor;
		Vector4 pieColor;


		PowerPie()
		{
			position = Vector2(0.f, 0.f);
			isActive = false;
		}
	};
	
	PowerPie m_PowerPie;

	Camera m_Camera;
	KeyboardControl m_Control;
	FlyControl m_FlyControl;
	QPointF m_PrevMousePos;

	QPointF m_MouseStartPos;
	QPointF m_MouseDir;
	QPointF m_MouseDirPrev;
	EventManager* m_EventManager;
	ResourceManager* m_ResourceManager;
	ToolManager m_ToolManager;

public:
	DXWidget(QWidget* parent = nullptr, Qt::WindowFlags flags = 0);


	virtual ~DXWidget() {}

	virtual void initialize(EventManager* p_EventManager, ResourceManager* p_ResourceManager, IGraphics* p_Graphics) = 0;
	virtual void uninitialize() = 0;

	virtual void render() {}
	virtual void present() {}

	virtual void onFrame(float p_DeltaTime);

protected:
	QPaintEngine* paintEngine() const { return nullptr; }
	virtual void onResize(unsigned int, unsigned int) = 0;

	void paintEvent(QPaintEvent* e) override;
	void resizeEvent(QResizeEvent* p_Event) override;
	void keyPressEvent(QKeyEvent* e) override;
	void keyReleaseEvent(QKeyEvent* e) override;
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;

signals:
    void CameraPositionChanged( Vector3 value );

private slots:
    void CameraPositionSet(Vector3 value);
};
