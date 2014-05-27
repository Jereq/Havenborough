#include "RotationTool.h"

RotationTool::RotationTool(IGraphics* p_Graphics, IPhysics* p_Physics, ResourceManager* p_ResourceManager)
	: m_Graphics(p_Graphics),
	m_Physics(p_Physics),
	m_ResourceManager(p_ResourceManager),
	m_YawCircleSelected(true),
	m_CurrentRadius(2.5f),
	m_CurrentYaw(0.f)
{
	m_CircleModelRes = m_ResourceManager->loadResource("model", "Circle");
	m_YawCircleModel = m_Graphics->createModelInstance("Circle");

	m_CircleVolumeRes = m_ResourceManager->loadResource("volume", "Circle");
	m_YawCircleBody = m_Physics->createBVInstance("Circle");
}

RotationTool::~RotationTool()
{
	m_Physics->releaseBody(m_YawCircleBody);
	m_Graphics->eraseModelInstance(m_YawCircleModel);

	m_ResourceManager->releaseResource(m_CircleModelRes);
	m_ResourceManager->releaseResource(m_CircleVolumeRes);
}

void RotationTool::render()
{
	//if (!m_YawCircleSelected)
	{
		m_Graphics->renderModel(m_YawCircleModel);
	}
}

void RotationTool::pick(BodyHandle p_PickedBody)
{
	if (p_PickedBody == m_YawCircleBody)
	{
		m_YawCircleSelected = true;
	}
}

void RotationTool::setSelection(Vector3 p_Position, float p_Radius)
{
	Vector3 scale = Vector3(p_Radius, p_Radius, p_Radius) * 0.4f;

	m_Graphics->setModelPosition(m_YawCircleModel, p_Position);
	m_Graphics->setModelScale(m_YawCircleModel, scale);

	m_Physics->setBodyPosition(m_YawCircleBody, p_Position);
	if (p_Radius != 0.f)
	{
		float relScale = p_Radius / m_CurrentRadius;
		m_Physics->setBodyScale(m_YawCircleBody, Vector3(relScale, relScale, relScale));
		m_CurrentRadius = p_Radius;
	}
}

void RotationTool::mouseReleased()
{
	m_YawCircleSelected = false;
}

void RotationTool::mouseMovement(QPointF p_Delta)
{
	if (m_YawCircleSelected)
	{
		float rotYaw = -p_Delta.x() * 2.f;

		Vector3 relRotation(rotYaw, 0.f, 0.f);
		m_CurrentYaw += rotYaw;
		while (m_CurrentYaw > 2 * PI)
		{
			m_CurrentYaw -= 2 * PI;
		}
		while (m_CurrentYaw < 0.f)
		{
			m_CurrentYaw += 2 * PI;
		}

		m_Graphics->setModelRotation(m_YawCircleModel, Vector3(m_CurrentYaw, 0.f, 0.f));
		m_Physics->setBodyRotation(m_YawCircleBody, Vector3(rotYaw, 0.f, 0.f));
		emit rotation(relRotation);
	}
}
