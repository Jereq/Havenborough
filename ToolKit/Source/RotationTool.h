#pragma once

#include <QObject>
#include <QPoint>

#include <IGraphics.h>
#include <IPhysics.h>

#ifndef Q_MOC_RUN
#include <ResourceManager.h>
#endif

class RotationTool : public QObject
{
	Q_OBJECT

private:
	float m_CurrentRadius;
	float m_CurrentYaw;

	int m_CircleModelRes;
	int m_CircleVolumeRes;

	bool m_YawCircleSelected;
	IGraphics::InstanceId m_YawCircleModel;
	BodyHandle m_YawCircleBody;

	IGraphics* m_Graphics;
	IPhysics* m_Physics;
	ResourceManager* m_ResourceManager;

public:
	RotationTool(IGraphics* p_Graphics, IPhysics* p_Physics, ResourceManager* p_ResourceManager);
	~RotationTool();

	void render();
	void pick(BodyHandle p_PickedBody);
	void setSelection(Vector3 p_Position, float p_Radius);

	void mouseReleased();
	void mouseMovement(QPointF p_Delta);

signals:
	void rotation(Vector3 p_Rotation);
};
