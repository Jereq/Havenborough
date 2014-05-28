#pragma once

#include <QObject>
#include <QPoint>

#include <IGraphics.h>
#include <IPhysics.h>

#ifndef Q_MOC_RUN
#include <ResourceManager.h>
#endif

class Camera;

class RotationTool : public QObject
{
	Q_OBJECT

private:
	enum class Tool
	{
		NONE,
		YAW,
		MOVE,
	};
	Tool m_SelectedTool;

	float m_CurrentRadius;
	float m_CurrentYaw;

	Vector3 m_SelectionPos;
	bool m_ObjectSelected;

	int m_CircleModelRes;
	int m_CircleVolumeRes;

	IGraphics::InstanceId m_YawCircleModel;
	BodyHandle m_YawCircleBody;

	BodyHandle m_MoveSphere;

	IGraphics* m_Graphics;
	IPhysics* m_Physics;
	ResourceManager* m_ResourceManager;
	const Camera* m_Camera;

public:
	RotationTool(IGraphics* p_Graphics, IPhysics* p_Physics, ResourceManager* p_ResourceManager, const Camera* p_Camera);
	~RotationTool();

	void render();
	void pick(BodyHandle p_PickedBody);
	void setSelection(Vector3 p_Position, float p_Radius);
	void deselect();

	void mouseReleased();
	void mouseMovement(QPointF p_PreviousPosition, QPointF p_NewPosition);

signals:
	void rotation(Vector3 p_Rotation);
	void translation(Vector3 p_Translation);
};