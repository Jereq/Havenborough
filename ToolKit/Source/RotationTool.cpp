#include "RotationTool.h"

#include "Camera.h"

RotationTool::RotationTool(IGraphics* p_Graphics, IPhysics* p_Physics, ResourceManager* p_ResourceManager, const Camera* p_Camera)
	: m_Graphics(p_Graphics),
	m_Physics(p_Physics),
	m_ResourceManager(p_ResourceManager),
	m_Camera(p_Camera),
	m_CurrentRadius(2.5f),
	m_MoveSphere(0),
	m_SelectedTool(Tool::NONE),
	m_ObjectSelected(false),
	m_YawCircleBody(0),
	m_PitchCircleBody(0),
	m_RollCircleBody(0)
{
	m_CircleModelRes = m_ResourceManager->loadResource("model", "Circle");
	m_YawCircleModel = m_Graphics->createModelInstance("Circle");
	m_PitchCircleModel = m_Graphics->createModelInstance("Circle");
	m_RollCircleModel = m_Graphics->createModelInstance("Circle");

	m_Graphics->setModelRotation(m_PitchCircleModel, Vector3(0.f, 0.f, PI * 0.5f));
	m_Graphics->setModelRotation(m_RollCircleModel, Vector3(0.f, PI * 0.5f, 0.f));

	m_CircleVolumeRes = m_ResourceManager->loadResource("volume", "Circle");
}

RotationTool::~RotationTool()
{
	if (m_YawCircleBody)
	{
		m_Physics->releaseBody(m_YawCircleBody);
		m_Physics->releaseBody(m_PitchCircleBody);
		m_Physics->releaseBody(m_RollCircleBody);
	}

	m_Graphics->eraseModelInstance(m_YawCircleModel);
	m_Graphics->eraseModelInstance(m_PitchCircleModel);
	m_Graphics->eraseModelInstance(m_RollCircleModel);

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

	case Tool::PITCH:
		m_Graphics->renderModel(m_PitchCircleModel);
		break;

	case Tool::ROLL:
		m_Graphics->renderModel(m_RollCircleModel);
		break;

	case Tool::NONE:
		m_Graphics->renderModel(m_YawCircleModel);
		m_Graphics->renderModel(m_PitchCircleModel);
		m_Graphics->renderModel(m_RollCircleModel);
		break;
	}
}

void RotationTool::pick(BodyHandle p_PickedBody)
{
	if (p_PickedBody == m_YawCircleBody)
	{
		m_SelectedTool = Tool::YAW;
	}
	else if (p_PickedBody == m_PitchCircleBody)
	{
		m_SelectedTool = Tool::PITCH;
	}
	else if (p_PickedBody == m_RollCircleBody)
	{
		m_SelectedTool = Tool::ROLL;
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

void RotationTool::setSelection(Vector3 p_Position, float p_Radius, Vector3 p_Rotation)
{
	m_SelectionPos = p_Position;
	m_ObjectSelected = true;
	m_CurrentRadius = p_Radius * 0.5f;
	m_CurrentRotation = p_Rotation;

	Vector3 scale = Vector3(m_CurrentRadius, m_CurrentRadius, m_CurrentRadius);

	m_Graphics->setModelPosition(m_YawCircleModel, p_Position);
	m_Graphics->setModelScale(m_YawCircleModel, scale);

	m_Graphics->setModelPosition(m_PitchCircleModel, p_Position);
	m_Graphics->setModelScale(m_PitchCircleModel, scale);

	m_Graphics->setModelPosition(m_RollCircleModel, p_Position);
	m_Graphics->setModelScale(m_RollCircleModel, scale);

	recreateBodies();
	updateModelRotations();

	if (m_MoveSphere)
	{
		m_Physics->releaseBody(m_MoveSphere);
	}

	m_MoveSphere = m_Physics->createSphere(0.f, true, m_SelectionPos, p_Radius * 0.8f);
}

void RotationTool::deselect()
{
	m_ObjectSelected = false;

	if (m_YawCircleBody)
	{
		m_Physics->releaseBody(m_YawCircleBody);
		m_Physics->releaseBody(m_PitchCircleBody);
		m_Physics->releaseBody(m_RollCircleBody);

		m_YawCircleBody = 0;
		m_PitchCircleBody = 0;
		m_RollCircleBody = 0;
	}

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

static float findCursorAngleOnPlane(const Camera* p_Camera, const Vector3& p_CenterPos, const Vector3& p_PlaneNormal, const QPointF& p_PrevPos, const QPointF& p_NewPos)
{
	using namespace DirectX;
	const XMVECTOR selectPos = XMLoadFloat3(&p_CenterPos);
	const XMVECTOR camPos = XMLoadFloat3(&p_Camera->getPosition());
	const XMVECTOR planeNormal = XMLoadFloat3(&p_PlaneNormal);
	const XMVECTOR plane = XMPlaneFromPointNormal(selectPos, planeNormal);
	const XMMATRIX view = XMMatrixLookToLH(camPos, XMLoadFloat3(&p_Camera->getForward()), XMLoadFloat3(&p_Camera->getUp()));
	const Vector2& viewportSize = p_Camera->getViewportSize();
	const XMMATRIX projection = XMMatrixPerspectiveFovLH(p_Camera->getFOV_Y(), viewportSize.x / viewportSize.y, 1.f, 1000.f);

	const XMVECTOR prevProjPos = XMVectorSet(p_PrevPos.x(), p_PrevPos.y(), 1.f, 1.f);
	const XMVECTOR prevProjected = XMVector3Unproject(prevProjPos, 0.f, 0.f, viewportSize.x, viewportSize.y, 0.f, 1.f, projection, view, XMMatrixIdentity());
	const XMVECTOR prevIntersect = XMPlaneIntersectLine(plane, camPos, prevProjected);

	const XMVECTOR curProjPos = XMVectorSet(p_NewPos.x(), p_NewPos.y(), 1.f, 1.f);
	const XMVECTOR curProjected = XMVector3Unproject(curProjPos, 0.f, 0.f, viewportSize.x, viewportSize.y, 0.f, 1.f, projection, view, XMMatrixIdentity());
	const XMVECTOR curIntersect = XMPlaneIntersectLine(plane, camPos, curProjected);

	const XMVECTOR prevOff = prevIntersect - selectPos;
	const XMVECTOR curOff = curIntersect - selectPos;

	const XMVECTOR prevLenSq = XMVector3LengthSq(prevOff);
	const XMVECTOR curLenSq = XMVector3LengthSq(curOff);

	if (prevLenSq.m128_f32[0] < 0.01f || curLenSq.m128_f32[0] < 0.01f)
	{
		return 0.f;
	}

	const XMVECTOR prevDir = prevOff / XMVectorSqrt(prevLenSq);
	const XMVECTOR curDir = curOff / XMVectorSqrt(curLenSq);

	const XMVECTOR cross = XMVector3Cross(prevDir, curDir);

	const float lengthSq = XMVector3LengthSq(cross).m128_f32[0];
	if (lengthSq < 0.0001f)
		return 0.f;

	const float length = sqrtf(lengthSq);
	const float dir = XMVector3Dot(cross, planeNormal).m128_f32[0];
	return (dir < 0.f) ? -asinf(length) : asinf(length);
}

void RotationTool::mouseMovement(QPointF p_PreviousPosition, QPointF p_NewPosition)
{
	if (!m_ObjectSelected)
	{
		return;
	}

	QPointF delta = p_NewPosition - p_PreviousPosition;

	switch (m_SelectedTool)
	{
	case Tool::YAW:
		{
			const float rotYaw = findCursorAngleOnPlane(m_Camera, m_SelectionPos, Vector3(0.f, 1.f, 0.f), p_PreviousPosition, p_NewPosition);

			m_CurrentRotation.x += rotYaw;
			while (m_CurrentRotation.x > 2 * PI)
			{
				m_CurrentRotation.x -= 2 * PI;
			}
			while (m_CurrentRotation.x < 0.f)
			{
				m_CurrentRotation.x += 2 * PI;
			}

			emit rotation(m_CurrentRotation);
		}
		break;
		
	case Tool::PITCH:
		{
			float rotPitch = findCursorAngleOnPlane(m_Camera, m_SelectionPos, Vector3(cosf(m_CurrentRotation.x), 0.f, -sinf(m_CurrentRotation.x)), p_PreviousPosition, p_NewPosition);

			m_CurrentRotation.y += rotPitch;
			while (m_CurrentRotation.y > 2 * PI)
			{
				m_CurrentRotation.y -= 2 * PI;
			}
			while (m_CurrentRotation.y < 0.f)
			{
				m_CurrentRotation.y += 2 * PI;
			}

			emit rotation(m_CurrentRotation);
		}
		break;

	case Tool::ROLL:
		{
			const float radiusXZ = cosf(m_CurrentRotation.y);
			float rotRoll = findCursorAngleOnPlane(m_Camera, m_SelectionPos, Vector3(sinf(m_CurrentRotation.x) * radiusXZ, -sinf(m_CurrentRotation.y), cosf(m_CurrentRotation.x) * radiusXZ), p_PreviousPosition, p_NewPosition);

			m_CurrentRotation.z += rotRoll;
			while (m_CurrentRotation.z > 2 * PI)
			{
				m_CurrentRotation.z -= 2 * PI;
			}
			while (m_CurrentRotation.z < 0.f)
			{
				m_CurrentRotation.z += 2 * PI;
			}

			emit rotation(m_CurrentRotation);
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

			emit translation(m_SelectionPos + trans);
		}
		break;
	}
	
	updateModelRotations();
	recreateBodies();
}

void RotationTool::recreateBodies()
{
	if (m_YawCircleBody)
	{
		m_Physics->releaseBody(m_YawCircleBody);
		m_Physics->releaseBody(m_PitchCircleBody);
		m_Physics->releaseBody(m_RollCircleBody);
	}

	m_YawCircleBody = m_Physics->createBVInstance("Circle");
	m_PitchCircleBody = m_Physics->createBVInstance("Circle");
	m_RollCircleBody = m_Physics->createBVInstance("Circle");

	updateBodyRotations();
	
	m_Physics->setBodyPosition(m_YawCircleBody, m_SelectionPos);
	m_Physics->setBodyPosition(m_PitchCircleBody, m_SelectionPos);
	m_Physics->setBodyPosition(m_RollCircleBody, m_SelectionPos);

	Vector3 scale(m_CurrentRadius, m_CurrentRadius, m_CurrentRadius);
	m_Physics->setBodyScale(m_YawCircleBody, scale);
	m_Physics->setBodyScale(m_PitchCircleBody, scale);
	m_Physics->setBodyScale(m_RollCircleBody, scale);
}

void RotationTool::updateBodyRotations()
{
	m_Physics->setBodyRotation(m_YawCircleBody, Vector3(m_CurrentRotation.x, 0.f, 0.f));
	m_Physics->setBodyRotation(m_PitchCircleBody, Vector3(m_CurrentRotation.x, m_CurrentRotation.y, PI * 0.5f));
	m_Physics->setBodyRotation(m_RollCircleBody, Vector3(m_CurrentRotation.x, m_CurrentRotation.y + PI * 0.5f, 0.f));
}

void RotationTool::updateModelRotations()
{
	m_Graphics->setModelRotation(m_YawCircleModel, Vector3(m_CurrentRotation.x, 0.f, 0.f));
	m_Graphics->setModelRotation(m_PitchCircleModel, Vector3(m_CurrentRotation.x, m_CurrentRotation.y, PI * 0.5f));
	m_Graphics->setModelRotation(m_RollCircleModel, Vector3(m_CurrentRotation.x, m_CurrentRotation.y + PI * 0.5f, 0.f));
}
