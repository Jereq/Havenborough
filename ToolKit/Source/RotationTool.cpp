#include "RotationTool.h"

#include "Camera.h"

RotationTool::RotationTool(IGraphics* p_Graphics, IPhysics* p_Physics, ResourceManager* p_ResourceManager, const Camera* p_Camera)
	: m_Graphics(p_Graphics),
	m_Physics(p_Physics),
	m_ResourceManager(p_ResourceManager),
	m_Camera(p_Camera),
	m_CurrentRadius(2.5f),
	m_CurrentYaw(0.f),
	m_MoveSphere(0),
	m_SelectedTool(Tool::NONE),
	m_ObjectSelected(false)
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

	if (m_MoveSphere)
	{
		m_Physics->releaseBody(m_MoveSphere);
		m_MoveSphere = 0;
	}
}

void RotationTool::render()
{
	if (!m_ObjectSelected)
		return;

	switch (m_SelectedTool)
	{
	case Tool::YAW:
		m_Graphics->renderModel(m_YawCircleModel);
		break;

	case Tool::NONE:
		m_Graphics->renderModel(m_YawCircleModel);
		break;
	}
}

void RotationTool::pick(BodyHandle p_PickedBody)
{
	if (p_PickedBody == m_YawCircleBody)
	{
		m_SelectedTool = Tool::YAW;
	}
	else if (p_PickedBody)
	{
		m_SelectedTool = Tool::MOVE;
	}
	else
	{
		m_SelectedTool = Tool::NONE;
	}
}

void RotationTool::setSelection(Vector3 p_Position, float p_Radius)
{
	m_SelectionPos = p_Position;
	m_ObjectSelected = true;

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

	if (m_MoveSphere)
	{
		m_Physics->releaseBody(m_MoveSphere);
	}

	m_MoveSphere = m_Physics->createSphere(0.f, true, m_SelectionPos, p_Radius * 0.8f);
}

void RotationTool::deselect()
{
	m_ObjectSelected = false;

	if (m_MoveSphere)
	{
		m_Physics->releaseBody(m_MoveSphere);
		m_MoveSphere = 0;
	}
}

void RotationTool::mouseReleased()
{
	m_SelectedTool = Tool::NONE;
}

void RotationTool::mouseMovement(QPointF p_PreviousPosition, QPointF p_NewPosition)
{
	QPointF delta = p_NewPosition - p_PreviousPosition;

	switch (m_SelectedTool)
	{
	case Tool::YAW:
		{
			float rotYaw = -delta.x() * 0.01f;

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
		break;

	case Tool::MOVE:
		{
			using namespace DirectX;
			XMVECTOR plane = XMPlaneFromPointNormal(XMLoadFloat3(&m_SelectionPos), XMLoadFloat3(&m_Camera->getForward()));
			XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&m_Camera->getPosition()), XMLoadFloat3(&m_Camera->getForward()), XMLoadFloat3(&m_Camera->getUp()));
			const Vector2& viewportSize = m_Camera->getViewportSize();
			XMMATRIX projection = XMMatrixPerspectiveFovLH(m_Camera->getFOV_Y(), viewportSize.x / viewportSize.y, 1.f, 1000.f);

			XMVECTOR prevProjPos = XMVectorSet(p_PreviousPosition.x(), p_PreviousPosition.y(), 1.f, 1.f);
			XMVECTOR prevProjected = XMVector3Unproject(prevProjPos, 0.f, 0.f, viewportSize.x, viewportSize.y, 0.f, 1.f, projection, view, XMMatrixIdentity());
			XMVECTOR prevIntersect = XMPlaneIntersectLine(plane, XMLoadFloat3(&m_Camera->getPosition()), prevProjected);

			XMVECTOR curProjPos = XMVectorSet(p_NewPosition.x(), p_NewPosition.y(), 1.f, 1.f);
			XMVECTOR curProjected = XMVector3Unproject(curProjPos, 0.f, 0.f, viewportSize.x, viewportSize.y, 0.f, 1.f, projection, view, XMMatrixIdentity());
			XMVECTOR curIntersect = XMPlaneIntersectLine(plane, XMLoadFloat3(&m_Camera->getPosition()), curProjected);

			XMVECTOR delta = curIntersect - prevIntersect;

			Vector3 trans;
			XMStoreFloat3(&trans, delta);

			emit translation(trans);
		}
		break;
	}
}
