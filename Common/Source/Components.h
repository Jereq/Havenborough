#pragma once

#include "Actor.h"
#include "ActorComponent.h"
#include "EventData.h"
#include "ResourceManager.h"
#include "CommonExceptions.h"
#include "XMLHelper.h"
#include "Utilities/Util.h"
#include "AnimationClip.h"


#include <IPhysics.h>

/**
 * Interface for a physics component.
 * 
 * Use for things like collisions and forces.
 */
class PhysicsInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 1;	/// Unique id
	Id getComponentId() const override
	{
		return m_ComponentId;
	}

	/**
	 * Get the body handle of the component.
	 *
	 * @return a body handle
	 */
	virtual BodyHandle getBodyHandle() const = 0;
	/**
	 * Get the body velocity of the component.
	 *
	 * @return velocity
	 */
	virtual Vector3 getVelocity() const = 0;
	/**
	 * Get if the components body is in the air.
	 *
	 * @return true if it the body is in the air otherwise false.
	 */
	virtual bool isInAir() const = 0;
	/**
	 * Get if the components body has landed .
	 *
	 * @return velocity
	 */
	virtual bool hasLanded() const = 0;
	/**
	 * Get if the components body is on something.
	 *
	 * @return true if it the body is on something otherwise false.
	 */
	virtual bool isOnSomething() const = 0;

	virtual void setScale(const Vector3& p_Scale, bool p_Pinned) = 0;
};

/**
 * Oriented bounding box component.
 */
class OBB_Component : public PhysicsInterface
{
private:
	BodyHandle m_Body;
	IPhysics* m_Physics;
	float m_Mass;
	bool m_Immovable;
	bool m_IsEdge;
	Vector3 m_OffsetPosition;
	Vector3 m_OffsetRotation;
	Vector3 m_Halfsize;
	Vector3 m_Scale;

public:
	~OBB_Component() override
	{
		m_Physics->releaseBody(m_Body);
	}

	/**
	 * Set the physics to use for the component.
	 *
	 * @param p_Physics the physics library to use
	 */
	void setPhysics(IPhysics* p_Physics)
	{
		m_Physics = p_Physics;
	}

	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		m_OffsetPosition = Vector3(0.f, 0.f, 0.f);
		m_OffsetRotation = Vector3(0.f, 0.f,0.f);
		m_Scale = Vector3(1.f, 1.f, 1.f);

		m_Halfsize = Vector3(1.f, 1.f, 1.f);
		const tinyxml2::XMLElement* size = p_Data->FirstChildElement("Halfsize");
		if (size)
		{
			m_Halfsize.x = size->FloatAttribute("x");
			m_Halfsize.y = size->FloatAttribute("y");
			m_Halfsize.z = size->FloatAttribute("z");
		}

		const tinyxml2::XMLElement* relPos = p_Data->FirstChildElement("OffsetPosition");
		if (relPos)
		{
			relPos->QueryAttribute("x", &m_OffsetPosition.x);
			relPos->QueryAttribute("y", &m_OffsetPosition.y);
			relPos->QueryAttribute("z", &m_OffsetPosition.z);
		}

		const tinyxml2::XMLElement* relRot = p_Data->FirstChildElement("OffsetRotation");
		if(relRot)
		{
			queryRotation(relRot, m_OffsetRotation);
		}

		m_Immovable = true;
		p_Data->QueryBoolAttribute("Immovable", &m_Immovable);

		m_Mass = 0.f;
		p_Data->QueryFloatAttribute("Mass", &m_Mass);

		m_IsEdge = false;
		p_Data->QueryBoolAttribute("IsEdge", &m_IsEdge);
	}

	void postInit() override
	{
		using namespace DirectX;
		XMFLOAT4X4 rotMat = m_Owner->getWorldMatrix();
		XMMATRIX mRotMat = XMMatrixTranspose(XMLoadFloat4x4(&rotMat));
		XMVECTOR pos = XMLoadFloat3(&XMFLOAT3(m_OffsetPosition));
		pos = XMVectorSetW(pos, 1.f);
		XMVECTOR rotPos = XMVector4Transform(pos, mRotMat);
		XMFLOAT3 fRotPos;
		XMStoreFloat3(&fRotPos, rotPos);

		m_Body = m_Physics->createOBB(m_Mass, m_Immovable, fRotPos, m_Halfsize, m_IsEdge);
		if(m_IsEdge)
			m_Physics->setBodyCollisionResponse(m_Body, false);


		Vector3 ownerRot = m_Owner->getRotation();
		XMMATRIX ownerRotation = XMMatrixRotationRollPitchYaw(ownerRot.y, ownerRot.x, ownerRot.z);
		XMMATRIX compRotation = XMMatrixRotationRollPitchYaw(m_OffsetRotation.y, m_OffsetRotation.x, m_OffsetRotation.z);
		XMMATRIX multRotation = compRotation * ownerRotation;
		XMFLOAT4X4 fMultRotation;
		XMStoreFloat4x4(&fMultRotation, multRotation);
		m_Physics->setBodyRotationMatrix(m_Body, fMultRotation);
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("OBBPhysics");
		p_Printer.PushAttribute("Immovable", m_Immovable);
		p_Printer.PushAttribute("Mass", m_Mass);
		p_Printer.PushAttribute("IsEdge", m_IsEdge);
		pushVector(p_Printer, "Halfsize", m_Halfsize);
		pushVector(p_Printer, "OffsetPosition", m_OffsetPosition);
		pushRotation(p_Printer, "OffsetRotation", m_OffsetRotation);
		p_Printer.CloseElement();
	}

	void onUpdate(float p_DeltaTime) override
	{
		(void)p_DeltaTime;

		if (!m_IsEdge)
		{
			using namespace DirectX;
			XMFLOAT4X4 rotMat = m_Owner->getWorldMatrix();
			XMMATRIX mRotMat = XMMatrixTranspose(XMLoadFloat4x4(&rotMat));
			XMVECTOR pos = XMLoadFloat3(&XMFLOAT3(m_OffsetPosition));
			pos = XMVectorSetW(pos, 0.f);
			XMVECTOR rotPos = XMVector4Transform(pos, mRotMat);
			XMFLOAT3 fRotPos;
			XMStoreFloat3(&fRotPos, rotPos);

			m_Owner->setPosition(m_Physics->getBodyPosition(m_Body) - fRotPos);
		}
	}

	void setPosition(Vector3 p_Position) override
	{
		using namespace DirectX;
		XMFLOAT4X4 rotMat = m_Owner->getWorldMatrix();
		XMMATRIX mRotMat = XMMatrixTranspose(XMLoadFloat4x4(&rotMat));
		XMVECTOR pos = XMLoadFloat3(&XMFLOAT3(m_OffsetPosition));
		pos = XMVectorSetW(pos, 0.f);
		XMVECTOR rotPos = XMVector4Transform(pos, mRotMat);
		XMFLOAT3 fRotPos;
		XMStoreFloat3(&fRotPos, rotPos);

		m_Physics->setBodyPosition(m_Body, p_Position + fRotPos);
	}

	void setRotation(Vector3 p_Rotation) override
	{
		using namespace DirectX;
		Vector3 ownerRot = p_Rotation;
		XMMATRIX ownerRotation = XMMatrixRotationRollPitchYaw(ownerRot.y, ownerRot.x, ownerRot.z);
		XMMATRIX compRotation = XMMatrixRotationRollPitchYaw(m_OffsetRotation.y, m_OffsetRotation.x, m_OffsetRotation.z);
		XMMATRIX multRotation = compRotation * ownerRotation;
		XMFLOAT4X4 fMultRotation;
		XMStoreFloat4x4(&fMultRotation, multRotation);
		m_Physics->setBodyRotationMatrix(m_Body, fMultRotation);
	}

	BodyHandle getBodyHandle() const override
	{
		return m_Body;
	}

	Vector3 getVelocity() const override
	{
		return m_Physics->getBodyVelocity(m_Body);
	}

	bool isInAir() const override
	{
		return m_Physics->getBodyInAir(m_Body);
	}
	bool hasLanded() const override
	{
		return m_Physics->getBodyLanded(m_Body);
	}
	bool isOnSomething() const
	{
		return m_Physics->getBodyOnSomething(m_Body);
	}
	void setScale(const Vector3& p_Scale, bool p_Pinned) override
	{
		if (p_Scale.x == 0.f || p_Scale.y == 0.f || p_Scale.z == 0.f)
		{
			return;
		}

		Vector3 relativeScale;
		relativeScale.x = p_Scale.x / m_Scale.x;
		relativeScale.y = p_Scale.y / m_Scale.y;
		relativeScale.z = p_Scale.z / m_Scale.z;
		m_Scale = p_Scale;
		m_Physics->setBodyScale(m_Body, relativeScale);

		if (!p_Pinned)
		{
			m_OffsetPosition.x *= relativeScale.x;
			m_OffsetPosition.y *= relativeScale.y;
			m_OffsetPosition.z *= relativeScale.z;
			setPosition(m_Owner->getPosition());
		}
	}
};

/**
 * Bounding sphere component.
 */
class CollisionSphereComponent : public PhysicsInterface
{
private:
	BodyHandle m_Body;
	IPhysics* m_Physics;
	Vector3 m_OffsetPositition;
	float m_Radius;
	float m_Mass;
	bool m_Immovable;
	bool m_CollisionResponse;

public:
	~CollisionSphereComponent() override
	{
		m_Physics->releaseBody(m_Body);
	}
	
	/**
	 * Set the physics to use for the component.
	 *
	 * @param p_Physics the physics library to use
	 */
	void setPhysics(IPhysics* p_Physics)
	{
		m_Physics = p_Physics;
	}

	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		m_OffsetPositition = Vector3(0.f, 0.f, 0.f);

		const tinyxml2::XMLElement* relPos = p_Data->FirstChildElement("OffsetPosition");
		if (relPos)
		{
			relPos->QueryAttribute("x", &m_OffsetPositition.x);
			relPos->QueryAttribute("y", &m_OffsetPositition.y);
			relPos->QueryAttribute("z", &m_OffsetPositition.z);
		}

		m_Immovable = true;
		p_Data->QueryBoolAttribute("Immovable", &m_Immovable);

		m_Radius = 1.f;
		p_Data->QueryFloatAttribute("Radius", &m_Radius);

		m_Mass = 0.f;
		p_Data->QueryAttribute("Mass", &m_Mass);

		m_CollisionResponse = true;
		p_Data->QueryBoolAttribute("CollisionResponse", &m_CollisionResponse);
	}

	void postInit() override
	{
		m_Body = m_Physics->createSphere(m_Mass, m_Immovable, m_Owner->getPosition() + m_OffsetPositition, m_Radius);
		m_Physics->setBodyCollisionResponse(m_Body, m_CollisionResponse);

	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("SpherePhysics");
		p_Printer.PushAttribute("Immovable", m_Immovable);
		p_Printer.PushAttribute("Radius", m_Radius);
		p_Printer.PushAttribute("Mass", m_Mass);
		p_Printer.PushAttribute("CollisionResponse", m_CollisionResponse);
		pushVector(p_Printer, "OffsetPosition", m_OffsetPositition);
		p_Printer.CloseElement();
	}

	void onUpdate(float p_DeltaTime) override
	{
		(void)p_DeltaTime;

		m_Owner->setPosition(m_Physics->getBodyPosition(m_Body) - m_OffsetPositition);
		Vector3 rotation = m_Owner->getRotation();
		m_Physics->setBodyRotation(m_Body, rotation);
	}

	void setPosition(Vector3 p_Position) override
	{
		m_Physics->setBodyPosition(m_Body, p_Position + m_OffsetPositition);
	}

	BodyHandle getBodyHandle() const override
	{
		return m_Body;
	}

	Vector3 getVelocity() const override
	{
		return m_Physics->getBodyVelocity(m_Body);
	}

	bool isInAir() const override
	{
		return m_Physics->getBodyInAir(m_Body);
	}
	bool hasLanded() const override
	{
		return m_Physics->getBodyLanded(m_Body);
	}
	bool isOnSomething() const
	{
		return m_Physics->getBodyOnSomething(m_Body);
	}
	void setScale(const Vector3& p_Scale, bool p_Pinned) override
	{
		(void)p_Pinned;
		m_Physics->setBodyScale(m_Body, p_Scale);
	}

	float getRadius() const
	{
		return m_Radius;
	}
};

/**
 * Axis-Aligned Bounding Box component.
 */
class AABB_Component : public PhysicsInterface
{
private:
	BodyHandle m_Body;
	IPhysics* m_Physics;
	Vector3 m_OffsetPositition;
	Vector3 m_Halfsize;
	float m_Mass;
	bool m_Immovable;
	bool m_IsEdge;
	bool m_RespondToCollision;

public:
	~AABB_Component() override
	{
		m_Physics->releaseBody(m_Body);
	}
	
	/**
	 * Set the physics to use for the component.
	 *
	 * @param p_Physics the physics library to use
	 */
	void setPhysics(IPhysics* p_Physics)
	{
		m_Physics = p_Physics;
	}

	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		m_OffsetPositition = Vector3(0.f, 0.f, 0.f);

		m_Halfsize = Vector3(1.f, 1.f, 1.f);
		const tinyxml2::XMLElement* size = p_Data->FirstChildElement("Halfsize");
		if (size)
		{
			m_Halfsize.x = size->FloatAttribute("x");
			m_Halfsize.y = size->FloatAttribute("y");
			m_Halfsize.z = size->FloatAttribute("z");
		}

		const tinyxml2::XMLElement* relPos = p_Data->FirstChildElement("OffsetPosition");
		if (relPos)
		{
			relPos->QueryAttribute("x", &m_OffsetPositition.x);
			relPos->QueryAttribute("y", &m_OffsetPositition.y);
			relPos->QueryAttribute("z", &m_OffsetPositition.z);
		}

		m_IsEdge = false;
		p_Data->QueryBoolAttribute("IsEdge", &m_IsEdge);
		m_RespondToCollision = true;
		p_Data->QueryBoolAttribute("CollisionResponse", &m_RespondToCollision);
		m_Mass = 0.f;
		p_Data->QueryFloatAttribute("Mass", &m_Mass);
		m_Immovable = true;
		p_Data->QueryBoolAttribute("Immovable", &m_Immovable);

	}

	void postInit() override
	{
		m_Body = m_Physics->createAABB(m_Mass, m_Immovable, m_Owner->getPosition() + m_OffsetPositition, m_Halfsize, m_IsEdge);
		m_Physics->setBodyCollisionResponse(m_Body, m_RespondToCollision);
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("AABBPhysics");
		p_Printer.PushAttribute("IsEdge", m_IsEdge);
		p_Printer.PushAttribute("Immovable", m_Immovable);
		p_Printer.PushAttribute("Mass", m_Mass);
		p_Printer.PushAttribute("CollisionResponse", m_RespondToCollision);
		pushVector(p_Printer, "Halfsize", m_Halfsize);
		pushVector(p_Printer, "OffsetPosition", m_OffsetPositition);
		p_Printer.CloseElement();
	}

	void onUpdate(float p_DeltaTime) override
	{
		(void)p_DeltaTime;

		m_Owner->setPosition(m_Physics->getBodyPosition(m_Body) - m_OffsetPositition);
	}

	void setPosition(Vector3 p_Position) override
	{
		m_Physics->setBodyPosition(m_Body, p_Position + m_OffsetPositition);
	}

	BodyHandle getBodyHandle() const override
	{
		return m_Body;
	}

	Vector3 getVelocity() const override
	{
		return m_Physics->getBodyVelocity(m_Body);
	}

	bool isInAir() const override
	{
		return m_Physics->getBodyInAir(m_Body);
	}
	bool hasLanded() const override
	{
		return m_Physics->getBodyLanded(m_Body);
	}
	bool isOnSomething() const
	{
		return m_Physics->getBodyOnSomething(m_Body);
	}
	void setScale(const Vector3& p_Scale, bool p_Pinned) override
	{
		(void)p_Pinned;
		m_Physics->setBodyScale(m_Body, p_Scale);
	}
};

/**
 * Bounding volume component based on a triangle mesh.
 */
class BoundingMeshComponent : public PhysicsInterface
{
private:
	BodyHandle m_Body;
	int m_MeshResourceId;
	IPhysics* m_Physics;
	ResourceManager* m_ResourceManager;
	Vector3 m_Scale;
	std::string m_MeshName;

public:
	~BoundingMeshComponent() override
	{
		m_Physics->releaseBody(m_Body);
		m_ResourceManager->releaseResource(m_MeshResourceId);
	}
	
	/**
	 * Set the physics to use for the component.
	 *
	 * @param p_Physics the physics library to use
	 */
	void setPhysics(IPhysics* p_Physics)
	{
		m_Physics = p_Physics;
	}
	/**
	 * Set the resource manager for the component.
	 *
	 * @param p_ResourceManager the resource manager to use
	 */
	void setResourceManager(ResourceManager* p_ResourceManager)
	{
		m_ResourceManager = p_ResourceManager;
	}

	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		const char* meshName = p_Data->Attribute("Mesh");

		if (!meshName)
		{
			throw CommonException("Collision component lacks mesh", __LINE__, __FILE__);
		}

		m_MeshName = meshName;
		m_MeshResourceId = m_ResourceManager->loadResource("volume", meshName);

		m_Scale = Vector3(1.f, 1.f, 1.f);
		const tinyxml2::XMLElement* scale = p_Data->FirstChildElement("Scale");
		if (scale)
		{
			m_Scale.x = scale->FloatAttribute("x");
			m_Scale.y = scale->FloatAttribute("y");
			m_Scale.z = scale->FloatAttribute("z");
		}
	}

	void postInit() override
	{
		m_Body = m_Physics->createBVInstance(m_MeshName.c_str());
		m_Physics->setBodyScale(m_Body, m_Scale);
		m_Physics->setBodyRotation(m_Body, m_Owner->getRotation());
		m_Physics->setBodyPosition(m_Body, m_Owner->getPosition());
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("MeshPhysics");
		p_Printer.PushAttribute("Mesh", m_MeshName.c_str());
		pushVector(p_Printer, "Scale", m_Scale);
		p_Printer.CloseElement();
	}

	void setPosition(Vector3 p_Position) override
	{
		m_Physics->setBodyPosition(m_Body, p_Position);
	}

	void setRotation(Vector3 p_Rotation) override
	{
		// Ugly fix for bad physics interface
		//m_Physics->setBodyRotation(m_Body, p_Rotation - m_Owner->getRotation());
		m_Physics->releaseBody(m_Body);
		m_Body = m_Physics->createBVInstance(m_MeshName.c_str());
		m_Physics->setBodyScale(m_Body, m_Scale);
		m_Physics->setBodyRotation(m_Body, p_Rotation);
		m_Physics->setBodyPosition(m_Body, m_Owner->getPosition());
	}

	BodyHandle getBodyHandle() const override
	{
		return m_Body;
	}

	Vector3 getVelocity() const override
	{
		return m_Physics->getBodyVelocity(m_Body);
	}

	bool isInAir() const override
	{
		return m_Physics->getBodyInAir(m_Body);
	}
	bool hasLanded() const override
	{
		return m_Physics->getBodyLanded(m_Body);
	}
	bool isOnSomething() const
	{
		return m_Physics->getBodyOnSomething(m_Body);
	}

	void setScale(const Vector3& p_Scale, bool p_Pinned) override
	{
		(void)p_Pinned;

		if (p_Scale.x == 0.f || p_Scale.y == 0.f || p_Scale.z == 0.f)
		{
			return;
		}

		Vector3 relativeScale;
		relativeScale.x = p_Scale.x / m_Scale.x;
		relativeScale.y = p_Scale.y / m_Scale.y;
		relativeScale.z = p_Scale.z / m_Scale.z;
		m_Scale = p_Scale;
		m_Physics->setBodyScale(m_Body, relativeScale);
	}
};

/**
 * Interface for model components.
 */
class ModelInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 2;	/// Unique id
	Id getComponentId() const override
	{
		return m_ComponentId;
	}
	virtual void setScale(Vector3 p_Scale) = 0;
	/**
	 * Update the scale of the model.
	 *
	 * @param p_CompName an identifier to keep track of the scale to keep track of it
	 * @param p_Scale the new scale
	 */
	virtual void updateScale(const std::string& p_CompName, Vector3 p_Scale) = 0;
	/**
	 * Remove a scale from the model.
	 *
	 * @param p_CompName an identifier of an existing scale
	 */
	virtual void removeScale(const std::string& p_CompName) = 0;
	/**
	 * Change a color tone for the model.
	 *
	 * @param p_ColorTone the color in RGB range 0.0f to 1.0f
	 */
	virtual void setColorTone(const Vector3 p_ColorTone) = 0;

	virtual void setOffset(const Vector3 p_Offset) = 0;

	virtual Vector3 getOffset() = 0;
};

/**
 * A standard model component.
 */
class ModelComponent : public ModelInterface
{
public:
	/**
	 * Id for separating different model components.
	 */
	typedef unsigned int ModelCompId;

private:
	ModelCompId m_Id;
	Vector3 m_BaseScale;
	Vector3 m_ColorTone;
	Vector3 m_Offset;
	bool NewPos;
	std::string m_MeshName;
	std::string m_Style;
	std::vector<std::pair<std::string, Vector3>> m_AppliedScales;

public:
	~ModelComponent() override
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new RemoveMeshEventData(m_Id)));
	}

	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		const char* mesh = p_Data->Attribute("Mesh");
		if (!mesh)
		{
			throw CommonException("Component lacks mesh", __LINE__, __FILE__);
		}

		m_MeshName = std::string(mesh);

		const char* style = p_Data->Attribute("Style");
		if (style)
			m_Style = style;

		m_BaseScale = Vector3(1.f, 1.f, 1.f);
		const tinyxml2::XMLElement* scale = p_Data->FirstChildElement("Scale");
		if (scale)
		{
			scale->QueryFloatAttribute("x", &m_BaseScale.x);
			scale->QueryFloatAttribute("y", &m_BaseScale.y);
			scale->QueryFloatAttribute("z", &m_BaseScale.z);
		}

		m_ColorTone = Vector3(1.f, 1.f, 1.f);
		const tinyxml2::XMLElement* tone = p_Data->FirstChildElement("ColorTone");
		if (tone)
		{
			tone->QueryFloatAttribute("x", &m_ColorTone.x);
			tone->QueryFloatAttribute("y", &m_ColorTone.y);
			tone->QueryFloatAttribute("z", &m_ColorTone.z);
		}

		m_Offset = Vector3(0,0,0);
		const tinyxml2::XMLElement* pos = p_Data->FirstChildElement("OffsetPosition");
		if (pos)
		{
			pos->QueryFloatAttribute("x", &m_Offset.x);
			pos->QueryFloatAttribute("y", &m_Offset.y);
			pos->QueryFloatAttribute("z", &m_Offset.z);
		}
	}
	void postInit() override
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new CreateMeshEventData(m_Id, m_MeshName,
			m_BaseScale, m_ColorTone, m_Style)));

		setPosition(m_Owner->getPosition());
		setRotation(m_Owner->getRotation());
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("Model");
		p_Printer.PushAttribute("Mesh", m_MeshName.c_str());
		if (!m_Style.empty())
			p_Printer.PushAttribute("Style", m_Style.c_str());
		pushVector(p_Printer, "Scale", m_BaseScale);
		pushVector(p_Printer, "ColorTone", m_ColorTone);
		pushVector(p_Printer, "OffsetPosition", m_Offset);
		p_Printer.CloseElement();
	}

	void setPosition(Vector3 p_Position) override
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateModelPositionEventData(m_Id, p_Position + m_Offset)));
	}

	void setOffset(const Vector3 p_Offset) override
	{
		Vector3 position = m_Owner->getPosition();
		m_Offset = p_Offset;
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateModelPositionEventData(m_Id, position + m_Offset)));
	}

	Vector3 getOffset() override
	{
		return m_Offset;
	}

	void setRotation(Vector3 p_Rotation) override
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateModelRotationEventData(m_Id, p_Rotation)));
	}

	void setScale(Vector3 p_Scale) override
	{
		m_BaseScale = p_Scale;
		calculateScale();
	}

	Vector3 getPosition()
	{
		return m_Owner->getPosition();
	}

	Vector3 getRotation()
	{
		return m_Owner->getRotation();
	}

	Vector3 getScale()
	{
		return m_BaseScale;
	}

	void updateScale(const std::string& p_CompName, Vector3 p_Scale) override
	{
		for (auto& scale : m_AppliedScales)
		{
			if (scale.first == p_CompName)
			{
				scale.second = p_Scale;
				calculateScale();
				return;
			}
		}

		m_AppliedScales.push_back(std::make_pair(p_CompName, p_Scale));

		calculateScale();
	}
	void removeScale(const std::string& p_CompName) override
	{
		for (auto& scale : m_AppliedScales)
		{
			if (scale.first == p_CompName)
			{
				std::swap(scale, m_AppliedScales.back());
				m_AppliedScales.pop_back();
				return;
			}
		}
	}

	void setColorTone(const Vector3 p_ColorTone) override
	{
		m_ColorTone = p_ColorTone;

		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new ChangeColorToneEvent(m_Id, m_ColorTone)));
	}

	/**
	 * Get the model component id from the component.
	 *
	 * @return unique id of the model
	 */
	ModelCompId getId() const
	{
		return m_Id;
	}

	/**
	 * Set the id of the model component.
	 *
	 * @param p_Id a new unique identifier
	 */
	void setId(ModelCompId p_Id)
	{
		m_Id = p_Id;
	}

	const std::string& getMeshName() const
	{
		return m_MeshName;
	}

private:
	void calculateScale()
	{
		Vector3 composedScale = m_BaseScale;
		for (const auto& scale : m_AppliedScales)
		{
			composedScale.x *= scale.second.x;
			composedScale.y *= scale.second.y;
			composedScale.z *= scale.second.z;
		}
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateModelScaleEventData(getId(), composedScale)));
	}

};

/**
 * Interface for movements.
 */
class MovementInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 3;	/// Unique id
	Id getComponentId() const override
	{
		return m_ComponentId;
	}

	/**
	 * Set the velocity of the movement.
	 *
	 * @param p_Velocity velocity in cm/s
	 */
	virtual void setVelocity(Vector3 p_Velocity) = 0;
	/**
	 * Get the velocity.
	 *
	 * @return the current velocity of the movement in cm/s
	 */
	virtual Vector3 getVelocity() const = 0;
	/**
	 * Set the velocity of rotation.
	 *
	 * @param p_RotVelocity the (yaw, pitch, roll) rotation velocity in radians/s
	 */
	virtual void setRotationalVelocity(Vector3 p_RotVelocity) = 0;
	/**
	 * Get the current rotational velocity.
	 *
	 * @return the (yaw, pitch, roll) rotation velocity in radians/s
	 */
	virtual Vector3 getRotationalVelocity() const = 0;
};

/**
 * Simple linear movement component implementation.
 */
class MovementComponent : public MovementInterface
{
private:
	Vector3 m_Velocity;
	Vector3 m_RotVelocity;

public:
	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		const tinyxml2::XMLElement* velElem = p_Data->FirstChildElement("Velocity");
		if (velElem)
		{
			m_Velocity.x = velElem->FloatAttribute("x");
			m_Velocity.y = velElem->FloatAttribute("y");
			m_Velocity.z = velElem->FloatAttribute("z");
		}
		else
		{
			m_Velocity = Vector3(0.f, 0.f, 0.f);
		}

		const tinyxml2::XMLElement* rotVelElem = p_Data->FirstChildElement("RotationalVelocity");
		if (rotVelElem)
		{
			m_RotVelocity.x = rotVelElem->FloatAttribute("x");
			m_RotVelocity.y = rotVelElem->FloatAttribute("y");
			m_RotVelocity.z = rotVelElem->FloatAttribute("z");
		}
		else
		{
			m_RotVelocity = Vector3(0.f, 0.f, 0.f);
		}
	}

	void onUpdate(float p_DeltaTime) override
	{
		const Vector3 pos = m_Owner->getPosition();
		m_Owner->setPosition(pos + m_Velocity * p_DeltaTime);

		const Vector3 rot = m_Owner->getRotation();
		m_Owner->setRotation(rot + m_RotVelocity * p_DeltaTime);
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("Movement");
		pushVector(p_Printer, "Velocity", m_Velocity);
		pushVector(p_Printer, "RotationalVelocity", m_RotVelocity);
		p_Printer.CloseElement();
	}

	void setVelocity(Vector3 p_Velocity) override
	{
		m_Velocity = p_Velocity;
	}
	Vector3 getVelocity() const override
	{
		return m_Velocity;
	}
	void setRotationalVelocity(Vector3 p_RotVelocity) override
	{
		m_RotVelocity = p_RotVelocity;
	}
	Vector3 getRotationalVelocity() const override
	{
		return m_RotVelocity;
	}
};

/**
 * Simple linear movement component implementation.
 */
class CircleMovementComponent : public MovementInterface
{
private:
	Vector3 m_CircleCenterPosition;
	float m_CircleAngle;
	float m_CircleRotationSpeed;
	float m_CircleRadius;

public:
	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		m_CircleCenterPosition = Vector3(0.f, 0.f, 0.f);
		m_CircleAngle = 0.f;
		m_CircleRotationSpeed = PI;
		m_CircleRadius = 100.f;

		const tinyxml2::XMLElement* centerElem = p_Data->FirstChildElement("CircleCenter");
		if (centerElem)
		{
			m_CircleCenterPosition.x = centerElem->FloatAttribute("x");
			m_CircleCenterPosition.y = centerElem->FloatAttribute("y");
			m_CircleCenterPosition.z = centerElem->FloatAttribute("z");
		}

		p_Data->QueryFloatAttribute("StartAngle", &m_CircleAngle);
		p_Data->QueryFloatAttribute("RotationSpeed", &m_CircleRotationSpeed);
		p_Data->QueryFloatAttribute("CircleRadius", &m_CircleRadius);
	}

	void postInit() override
	{
		onUpdate(0.f);
	}

	void onUpdate(float p_DeltaTime) override
	{
		m_CircleAngle += m_CircleRotationSpeed * p_DeltaTime;
		Vector3 newPos = m_CircleCenterPosition;
		newPos.x += cos(m_CircleAngle) * m_CircleRadius;
		newPos.z += -sin(m_CircleAngle) * m_CircleRadius;
		Vector3 newRot(m_CircleAngle, 0.f, m_CircleAngle);

		m_Owner->setPosition(newPos);
		m_Owner->setRotation(newRot);
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("CircleMovement");
		p_Printer.PushAttribute("StartAngle", m_CircleAngle);
		p_Printer.PushAttribute("RotationSpeed", m_CircleRotationSpeed);
		p_Printer.PushAttribute("CircleRadius", m_CircleRadius);
		pushVector(p_Printer, "CircleCenter", m_CircleCenterPosition);
		p_Printer.CloseElement();
	}

	void setVelocity(Vector3 p_Velocity) override
	{
		(void)p_Velocity;
	}
	Vector3 getVelocity() const override
	{
		Vector3 newVel;
		newVel.x = -sin(m_CircleAngle) * m_CircleRadius * m_CircleRotationSpeed;
		newVel.y = 0.f;
		newVel.z = -cos(m_CircleAngle) * m_CircleRadius * m_CircleRotationSpeed;
		return newVel;
	}
	void setRotationalVelocity(Vector3 p_RotVelocity) override
	{
		(void)p_RotVelocity;
	}
	Vector3 getRotationalVelocity() const override
	{
		return Vector3(m_CircleRotationSpeed, 0.f, m_CircleRotationSpeed);
	}
	Vector3 getCenterPosition() const
	{
		return m_CircleCenterPosition;
	}
	float getRadius() const
	{
		return m_CircleRadius;
	}
};

/**
 * Interface for visibly pulsing an actor.
 */
class PulseInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 4;	/// Unique id
	virtual Id getComponentId() const override
	{
		return m_ComponentId;
	}
	/**
	 * Activate a single pulse.
	 */
	virtual void pulseOnce() = 0;
};

/**
 * Scaling component implementation of pulsing.
 */
class PulseComponent : public PulseInterface
{
private:
	float m_PulseLength;
	float m_PulseStrength;
	float m_CurrentTime;

public:
	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		m_PulseLength = 1.f;
		p_Data->QueryFloatAttribute("Length", &m_PulseLength);
		
		m_PulseStrength = 0.5f;
		p_Data->QueryAttribute("Strength", &m_PulseLength);

		m_CurrentTime = m_PulseLength;
	}

	void onUpdate(float p_DeltaTime) override
	{
		if (m_CurrentTime < m_PulseLength)
		{
			m_CurrentTime += p_DeltaTime;
			if (m_CurrentTime < m_PulseLength)
			{
				static const float pi = 3.141592f;
				Vector3 appliedScale = Vector3(1.f, 1.f, 1.f) * (1.f + sin(pi * m_CurrentTime / m_PulseLength) * m_PulseStrength);

				std::shared_ptr<ModelInterface> modelComp(m_Owner->getComponent<ModelInterface>(2));
				if (modelComp)
				{
					modelComp->updateScale("PulseComp", appliedScale);
				}
			}
			else
			{
				std::shared_ptr<ModelInterface> modelComp(m_Owner->getComponent<ModelInterface>(2));
				if (modelComp)
				{
					modelComp->removeScale("PulseComp");
				}
			}
		}
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("Pulse");
		p_Printer.PushAttribute("Length", m_PulseLength);
		p_Printer.PushAttribute("Strength", m_PulseStrength);
		p_Printer.CloseElement();
	}

	void pulseOnce() override
	{
		m_CurrentTime = 0.f;
	}
};

/**
 * Interface for components shining light.
 */
class LightInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 5;	/// Unique id
	Id getComponentId() const override
	{
		return m_ComponentId;
	}
};

/**
 * Light component implementation providing point, spot and directional lights.
 */
class LightComponent : public LightInterface
{
private:
	LightClass m_Light;
	Vector3 m_Offset;

public:
	~LightComponent() override
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new RemoveLightEventData(m_Light.id)));
	}

	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		int tempId = m_Light.id;

		if (p_Data->Attribute("Type", "Point"))
		{
			Vector3 position(0.f, 0.f, 0.f);
			Vector3 color(1.f, 1.f, 1.f);
			float range = 2000.f;

			p_Data->QueryAttribute("Range", &range);

			const tinyxml2::XMLElement* pos = p_Data->FirstChildElement("Position");
			if (pos)
			{
				pos->QueryAttribute("x", &position.x);
				pos->QueryAttribute("y", &position.y);
				pos->QueryAttribute("z", &position.z);
			}

			const tinyxml2::XMLElement* col = p_Data->FirstChildElement("Color");
			if (col)
			{
				col->QueryAttribute("r", &color.x);
				col->QueryAttribute("g", &color.y);
				col->QueryAttribute("b", &color.z);
			}

			m_Light = LightClass::createPointLight(position, range, color);
			m_Light.id = tempId;
		}
		else if (p_Data->Attribute("Type", "Spot"))
		{
			Vector3 position(0.f, 0.f, 0.f);
			Vector3 direction(0.f, -1.f, 0.f);
			Vector3 color(1.f, 1.f, 1.f);
			Vector2 angles(1.f, 1.f);
			float range = 2000.f;

			p_Data->QueryAttribute("Range", &range);

			/*const tinyxml2::XMLElement* pos = p_Data->FirstChildElement("Position");
			if (pos)
			{
				pos->QueryAttribute("x", &position.x);
				pos->QueryAttribute("y", &position.y);
				pos->QueryAttribute("z", &position.z);
			}*/
			m_Offset = Vector3(0,0,0);

			const tinyxml2::XMLElement* dir = p_Data->FirstChildElement("Direction");
			if (dir)
			{
				dir->QueryAttribute("x", &direction.x);
				dir->QueryAttribute("y", &direction.y);
				dir->QueryAttribute("z", &direction.z);
			}

			const tinyxml2::XMLElement* col = p_Data->FirstChildElement("Color");
			if (col)
			{
				col->QueryAttribute("r", &color.x);
				col->QueryAttribute("g", &color.y);
				col->QueryAttribute("b", &color.z);
			}

			const tinyxml2::XMLElement* ang = p_Data->FirstChildElement("Angles");
			if (ang)
			{
				ang->QueryAttribute("min", &angles.x);
				ang->QueryAttribute("max", &angles.y);
			}

			m_Light = LightClass::createSpotLight(position, direction, angles, range, color);
			m_Light.id = tempId;
		}
		else if (p_Data->Attribute("Type", "Directional"))
		{
			Vector3 direction(0.f, -1.f, 0.f);
			Vector3 color(1.f, 1.f, 1.f);
			float intensity;
			const tinyxml2::XMLElement* dir = p_Data->FirstChildElement("Direction");
			if (dir)
			{
				dir->QueryAttribute("x", &direction.x);
				dir->QueryAttribute("y", &direction.y);
				dir->QueryAttribute("z", &direction.z);
			}
			const tinyxml2::XMLElement* intens = p_Data->FirstChildElement("Intensity");
			if(intens)
			{
				intens->QueryAttribute("Intensity", &intensity);
			}
			const tinyxml2::XMLElement* col = p_Data->FirstChildElement("Color");
			if (col)
			{
				col->QueryAttribute("r", &color.x);
				col->QueryAttribute("g", &color.y);
				col->QueryAttribute("b", &color.z);
			}
			
			m_Light = LightClass::createDirectionalLight(direction, color, intensity);
			m_Light.id = tempId;
		}
		else
		{
			throw CommonException("XML Light description missing valid type", __LINE__, __FILE__);
		}
	}
	void postInit() override
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new LightEventData(m_Light)));
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("Light");
		switch (m_Light.type)
		{
		case LightClass::Type::DIRECTIONAL:
			{
				p_Printer.PushAttribute("Type", "Directional");
				pushVector(p_Printer, "Direction", m_Light.direction);
				p_Printer.OpenElement("Intensity");
				p_Printer.PushAttribute("Intensity", m_Light.intensity);
				p_Printer.CloseElement();
			}
			break;

		case LightClass::Type::POINT:
			{
				p_Printer.PushAttribute("Type", "Point");
				p_Printer.PushAttribute("Range", m_Light.range);
				pushVector(p_Printer, "Position", m_Light.position);
			}
			break;

		case LightClass::Type::SPOT:
			{
				p_Printer.PushAttribute("Type", "Spot");
				p_Printer.PushAttribute("Range", m_Light.range);
				pushVector(p_Printer, "Position", m_Light.position);
				pushVector(p_Printer, "Direction", m_Light.direction);
				p_Printer.OpenElement("Angles");
				p_Printer.PushAttribute("min", m_Light.spotlightAngles.x);
				p_Printer.PushAttribute("max", m_Light.spotlightAngles.y);
				p_Printer.CloseElement();
			}
			break;

		default:
			p_Printer.PushAttribute("Type", "Unknown");
			break;
		};
		pushColor(p_Printer, "Color", m_Light.color);
		p_Printer.CloseElement();
	}

	//void setPosition(Vector3 p_Position)
	//{
	//	m_Light.position = p_Position;
	//	m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateLightPositionEventData(m_Light.id, p_Position)));
	//}

	void setPosition(Vector3 p_Position) override
	{
		m_Light.position = p_Position + m_Offset;
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateLightPositionEventData(m_Light.id, m_Light.position)));
	}

	const Vector3& getPosition() const
	{
		return m_Light.position;
	}

	void setDirection(Vector3 p_Direction)
	{
		m_Light.direction = p_Direction;
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateLightDirectionEventData(m_Light.id, p_Direction)));
	}

	const Vector3& getDirection() const
	{
		return m_Light.direction;
	}

	void setColor(Vector3 p_Color)
	{
		m_Light.color = p_Color;
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateLightColorEventData(m_Light.id, p_Color)));
	}

	const Vector3& getColor() const
	{
		return m_Light.color;
	}

	void setSpotLightAngles(Vector2 p_Angles)
	{
		m_Light.spotlightAngles = p_Angles;
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateLightAngleEventData(m_Light.id, p_Angles)));
	}

	const Vector2& getSpotLightAngles() const
	{
		return m_Light.spotlightAngles;
	}

	void setRange(float p_Range)
	{
		m_Light.range = p_Range;
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateLightRangeEventData(m_Light.id, p_Range)));
	}

	const float& getRange() const
	{
		return m_Light.range;
	}

	void setIntensity(float p_Intensity)
	{
		m_Light.intensity = p_Intensity;
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateLightRangeEventData(m_Light.id, p_Intensity)));
	}

	const float& getIntensity() const
	{
		return m_Light.intensity;
	}

	/**
	 * Get the unique id of the light.
	 *
	 * @return the lights unique identifier
	 */
	LightClass::Id getId() const
	{
		return m_Light.id;
	}

	/**
	 * Set a new unique identifier for the light.
	 *
	 * @param p_Id the light's id
	 */
	void setId(LightClass::Id p_Id)
	{
		m_Light.id = p_Id;
	}

	const LightClass::Type getType() const
	{
		return m_Light.type;
	}
};

class LookInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 6;	/// Unique id
	virtual Id getComponentId() const override
	{
		return m_ComponentId;
	}
	virtual Vector3 getLookPosition() const = 0;
	virtual Vector3 getLookForward() const = 0;
	virtual void setLookForward(Vector3 p_Forward) = 0;
	virtual Vector3 getLookUp() const = 0;
	virtual void setLookUp(Vector3 p_Up) = 0;
	virtual Vector3 getLookRight() const = 0;
	virtual DirectX::XMFLOAT4X4 getRotationMatrix() const = 0;
};

class ParticleInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 7;	/// Unique id
	virtual Id getComponentId() const override
	{
		return m_ComponentId;
	}

	virtual void setBaseColor(Vector4 p_NewBaseColor) = 0;
	//virtual void setRotation(Vector3 p_NewRotation) = 0;
	//virtual void setOffsetPosition(Vector3 p_NewPosition) = 0;

};

class ParticleComponent : public ParticleInterface
{
private:
	unsigned int m_ParticleId;
	std::string m_EffectName;
	Vector4 m_BaseColor;
	Vector3 m_OffsetPosition;
	Vector3 m_Rotation;

public:
	~ParticleComponent()
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new RemoveParticleEventData(m_ParticleId)));
	}

	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		const char* effectName = p_Data->Attribute("Effect");
		if (!effectName)
		{
			throw CommonException("Missing effect name", __LINE__, __FILE__);
		}
		m_BaseColor = Vector4(-1.f, -1.f, -1.f, -1.f);
		queryColor(p_Data->FirstChildElement("BaseColor"), m_BaseColor);
		queryVector(p_Data->FirstChildElement("OffsetPosition"), m_OffsetPosition);
		queryRotation(p_Data->FirstChildElement("Rotation"), m_Rotation);

		m_EffectName = effectName;
	}

	void postInit() override
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new CreateParticleEventData(m_ParticleId, m_EffectName, m_Owner->getPosition())));
		if (m_BaseColor.x != -1.f)
		{
			setBaseColor(m_BaseColor);
		}
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("Particle");
		p_Printer.PushAttribute("Effect", m_EffectName.c_str());
		if (m_BaseColor.x != -1.f)
		{
			pushColor(p_Printer, "BaseColor", m_BaseColor);
		}
		p_Printer.CloseElement();
	}

	void setPosition(Vector3 p_Position)
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateParticlePositionEventData(m_ParticleId, p_Position)));
	}

	void setRotation(Vector3 p_Rotation) override
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateParticleRotationEventData(m_ParticleId, p_Rotation)));
	}

	void setBaseColor(Vector4 p_BaseColor) override
	{
		m_BaseColor = p_BaseColor;
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new UpdateParticleBaseColorEventData(m_ParticleId, p_BaseColor)));
	}

	/**
	 * Get the unique id of the particle effect.
	 *
	 * @return the particle effect's unique identifier
	 */
	unsigned int getId() const
	{
		return m_ParticleId;
	}

	/**
	 * Set a new unique identifier for the particle effect.
	 *
	 * @param p_Id the particle effect's id
	 */
	void setId(unsigned int p_Id)
	{
		m_ParticleId = p_Id;
	}

	const std::string& getEffectName() const
	{
		return m_EffectName;
	}
};

class AnimationInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 8;	/// Unique id
	virtual Id getComponentId() const override
	{
		return m_ComponentId;
	}
	
	/**
	 * Poll the animation component to play a new animation clip.
	 * @param p_AnimationName, the name of the wanted clip.
	 * @param p_Override, specify if the clip should override active clips.
	 */
	virtual void playAnimation(std::string p_AnimationName, bool p_Override) = 0;

	/**
	 * Poll the animation component to queue a new animation clip.
	 * @param p_AnimationName, the name of the wanted clip.
	 */
	virtual void queueAnimation(std::string p_AnimationName) = 0;

	/**
	 * Change the influence of an animation track pair.
	 * @param p_Track, first track number in the pair.
	 * @param p_Weight, the wanted influence.
	 */
	virtual void changeAnimationWeight(int p_Track, float p_Weight) = 0;

	/**
	 * Calculate 3-joint IK to reach for a specified point.
	 * @param p_GroupName, the wanted IK-group.
	 * @param p_Target, 3D point to reach for.
	 */
	virtual void applyIK_ReachPoint(const std::string& p_GroupName, Vector3 p_Target, float p_Weight) = 0;

	/**
	 * @param p_JointName, the name of the joint to get the position of.
	 * @return the joint position in World space.
	 */
	virtual DirectX::XMFLOAT3 getJointPos(const std::string& p_JointName) = 0;

	/**
	 * Poll the animation to return a specified animation path for climbing.
	 * @param p_AnimationId, the name of the animation path.
	 * @return the wanted animation path.
	 */
	virtual const AnimationPath getAnimationData(std::string p_AnimationId) const = 0;

	/**
	 * Automaticly plays the specified climb animation and locks all other animations from running.
	 * @param p_ClimbId, the name of the wanted climb animation.
	 */
	virtual void playClimbAnimation(std::string p_ClimbID) = 0;

	/**
	 * Unlock the animations.
	 */
	virtual void resetClimbState() = 0;

	/**
	 * Update the needed data for climb IK.
	 * @param p_EdgeOrientation, the calculated orientation of the edge.
	 * @param p_CenterReachPos, the center position that the IK uses.
	 */
	virtual void updateIKData(Vector3 p_EdgeOrientation, Vector3 p_CenterReachPos, std::string p_grabName) = 0;

	/**
	 * The animation component needs physics for some of its calculations.
	 * @param p_Physics, a pointer to the physics engine.
	 */
	virtual void setPhysics(IPhysics *p_Physics) = 0;

	virtual void applyLookAtIK(const std::string& p_GroupName, const DirectX::XMFLOAT3& p_Target, float p_MaxAngle) = 0;

	virtual void setLookAtPoint(const DirectX::XMFLOAT3& p_Target) = 0;
	virtual bool getLanding() = 0;
	virtual DirectX::XMFLOAT4X4 getViewDirection(std::string p_Joint) = 0;
};

class SpellInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 9;	/// Unique id
	virtual Id getComponentId() const override
	{
		return m_ComponentId;
	}

	/**
	 * Get the body handle of the component.
	 *
	 * @return a body handle
	 */
	virtual BodyHandle getBodyHandle() const = 0;
};

class MovementControlInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 10;
	virtual Id getComponentId() const override
	{
		return m_ComponentId;
	}

	virtual void move(float p_DeltaTime) = 0;

	virtual Vector3 getLocalDirection() const = 0;
	virtual void setLocalDirection(const Vector3& p_Direction) = 0;
	virtual float getMaxSpeed() const = 0;
	virtual void setMaxSpeed(float p_Speed) = 0;
	virtual float getMaxSpeedDefault() const = 0;
};

/**
 * Interface for model components.
 */
class TextInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 11;	/// Unique id
	Id getComponentId() const override
	{
		return m_ComponentId;
	}

	virtual void setId(unsigned int p_ComponentId) = 0;

	virtual unsigned int getId() = 0;
};


class TextComponent : public TextInterface
{
private:
	std::string m_Text;
	std::string m_Font;
	float m_FontSize;
	Vector4 m_FontColor;
	Vector4 m_BackgroundColor;
	Vector3 m_OffsetPosition;
	float m_Scale;
	float m_Rotation;

	Vector3 m_WorldPosition;

	unsigned int m_ComponentId;

public:
	~TextComponent() override
	{
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new removeWorldTextEventData(m_ComponentId)));
	}

	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		const char* text = p_Data->Attribute("Text");
		if (!text)
		{
			throw CommonException("Component lacks text", __LINE__, __FILE__);
		}

		m_Text = std::string(text);

		m_Font = "Verdana";
		m_Scale = 1.f;
		m_Rotation = 0.f;
		m_FontSize = 12.f;
		const tinyxml2::XMLElement* textSettings = p_Data->FirstChildElement("TextSettings");
		if (textSettings)
		{
			const char* font = textSettings->Attribute("Font");
			if(font != nullptr)
				m_Font = font;

			textSettings->QueryFloatAttribute("FontSize", &m_FontSize);
			textSettings->QueryFloatAttribute("Scale", &m_Scale);
			textSettings->QueryFloatAttribute("Rotation", &m_Rotation);
		}

		m_BackgroundColor = Vector4(1.f, 1.f, 1.f, 1.f);
		const tinyxml2::XMLElement* backtone = p_Data->FirstChildElement("BackgroundColor");
		if (backtone)
		{
			backtone->QueryFloatAttribute("r", &m_BackgroundColor.x);
			backtone->QueryFloatAttribute("g", &m_BackgroundColor.y);
			backtone->QueryFloatAttribute("b", &m_BackgroundColor.z);
			backtone->QueryFloatAttribute("a", &m_BackgroundColor.w);
		}

		m_FontColor = Vector4(1.f, 1.f, 1.f, 1.f);
		const tinyxml2::XMLElement* tone = p_Data->FirstChildElement("FontColor");
		if (tone)
		{
			tone->QueryFloatAttribute("r", &m_FontColor.x);
			tone->QueryFloatAttribute("g", &m_FontColor.y);
			tone->QueryFloatAttribute("b", &m_FontColor.z);
			tone->QueryFloatAttribute("a", &m_FontColor.w);
		}

		m_OffsetPosition = Vector3(0.f, 0.f, 0.f);
		const tinyxml2::XMLElement* offPos = p_Data->FirstChildElement("OffsetPosition");
		if (tone)
		{
			offPos->QueryFloatAttribute("x", &m_OffsetPosition.x);
			offPos->QueryFloatAttribute("y", &m_OffsetPosition.y);
			offPos->QueryFloatAttribute("z", &m_OffsetPosition.z);
		}

	}
	void postInit() override
	{
		m_WorldPosition = m_Owner->getPosition() + m_OffsetPosition;
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new createWorldTextEventData(m_Text,
			m_Font, m_FontSize, m_FontColor, m_BackgroundColor, m_WorldPosition, m_Scale, m_Rotation, m_ComponentId)));
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("TextComponent");
		p_Printer.PushAttribute("Text", m_Text.c_str());
		p_Printer.OpenElement("TextSettings");
		p_Printer.PushAttribute("Font", m_Font.c_str());
		p_Printer.PushAttribute("FontSize", m_FontSize);
		p_Printer.PushAttribute("Scale", m_Scale);
		p_Printer.PushAttribute("Rotation", m_Rotation);
		p_Printer.CloseElement();
		pushColor(p_Printer, "BackgroundColor", m_BackgroundColor);
		pushColor(p_Printer, "FontColor", m_FontColor);
		pushVector(p_Printer, "OffsetPosition", m_OffsetPosition);
		p_Printer.CloseElement();
	}

	void onUpdate(float p_DeltaTime) override
	{
		(void)p_DeltaTime;

		m_WorldPosition = m_Owner->getPosition() + m_OffsetPosition;
		m_Owner->getEventManager()->queueEvent(IEventData::Ptr(new updateWorldTextPositionEventData(m_ComponentId,m_WorldPosition)));
	}

	void setId(unsigned int p_ComponentId)
	{
		m_ComponentId = p_ComponentId;
	}

	unsigned int getId()
	{
		return m_ComponentId;
	}
};

class OffsetCalculationInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 12; /// Unique id
	Id getComponentId() const override
	{
		return m_ComponentId;
	}
};

class ModelSinOffsetComponent : public OffsetCalculationInterface
{
private:
	Vector3 m_Position;
	Vector3 m_Offset;
	float m_Time;
	std::weak_ptr<ModelInterface> m_Model;
public:

	~ModelSinOffsetComponent() override
	{

	}

	void initialize(const tinyxml2::XMLElement* p_Data) override
	{
		m_Time = 0;
		m_Offset = Vector3(0,0,0);
		p_Data->QueryAttribute("StartTime", &m_Time);
		queryVector(p_Data->FirstChildElement("Offset"), m_Offset);
	}
	
	void postInit() override
	{
		m_Model = m_Owner->getComponent<ModelInterface>(ModelInterface::m_ComponentId);
		if(!m_Model.lock())
		{
			return;
		}
		m_Position = m_Model.lock()->getOffset();
	}

	void serialize(tinyxml2::XMLPrinter& p_Printer) const override
	{
		p_Printer.OpenElement("ModelSinOffset");
		p_Printer.PushAttribute("StartTime", m_Time);
		pushVector(p_Printer, "Offset", m_Offset);
		p_Printer.CloseElement();
	}

	void onUpdate(float p_DeltaTime) override
	{
		m_Time += p_DeltaTime;

		float sinus = std::sin(m_Time);
		Vector3 result = m_Position + m_Offset * sinus;
		if(!m_Model.lock())
		{
			return;
		}
		m_Model.lock()->setOffset(result);
	}
};


class SoundInterface : public ActorComponent
{
public:
	static const Id m_ComponentId = 13; /// Unique id
	Id getComponentId() const override
	{
		return m_ComponentId;
	}
};
