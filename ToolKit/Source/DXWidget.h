#pragma once

#include <QWidget>

#include "Camera.h"
#include "FlyControl.h"
#include "KeyboardControl.h"

class EventManager;
class IGraphics;
class ResourceManager;

class DXWidget : public QWidget
{
	Q_OBJECT

protected:
	Camera m_Camera;
	KeyboardControl m_Control;
	FlyControl m_FlyControl;
	QPointF m_PrevMousePos;

	EventManager* m_EventManager;
	ResourceManager* m_ResourceManager;

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
