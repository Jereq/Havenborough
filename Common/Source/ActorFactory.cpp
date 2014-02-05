#include "ActorFactory.h"
#include "CommonExceptions.h"
#include "Components.h"
#include "HumanAnimationComponent.h"
#include "XMLHelper.h"

ActorFactory::ActorFactory(unsigned int p_BaseActorId)
	:	m_LastActorId(p_BaseActorId),
		m_LastModelComponentId(0),
		m_LastLightComponentId(0),
		m_LastParticleComponentId(0),
		m_Physics(nullptr)
{
	m_ComponentCreators["OBBPhysics"] = std::bind(&ActorFactory::createOBBComponent, this);
	m_ComponentCreators["AABBPhysics"] = std::bind(&ActorFactory::createAABBComponent, this);
	m_ComponentCreators["SpherePhysics"] = std::bind(&ActorFactory::createCollisionSphereComponent, this);
	m_ComponentCreators["MeshPhysics"] = std::bind(&ActorFactory::createBoundingMeshComponent, this);
	m_ComponentCreators["Model"] = std::bind(&ActorFactory::createModelComponent, this);
	m_ComponentCreators["Movement"] = std::bind(&ActorFactory::createMovementComponent, this);
	m_ComponentCreators["CircleMovement"] = std::bind(&ActorFactory::createCircleMovementComponent, this);
	m_ComponentCreators["Pulse"] = std::bind(&ActorFactory::createPulseComponent, this);
	m_ComponentCreators["Light"] = std::bind(&ActorFactory::createLightComponent, this);
	m_ComponentCreators["Particle"] = std::bind(&ActorFactory::createParticleComponent, this);
	m_ComponentCreators["Look"] = std::bind(&ActorFactory::createLookComponent, this);
	m_ComponentCreators["HumanAnimation"] = std::bind(&ActorFactory::createHumanAnimationComponent, this);
}

void ActorFactory::setPhysics(IPhysics* p_Physics)
{
	m_Physics = p_Physics;
}

void ActorFactory::setEventManager(EventManager* p_EventManager)
{
	m_EventManager = p_EventManager;
}

void ActorFactory::setResourceManager(ResourceManager* p_ResourceManager)
{
	m_ResourceManager = p_ResourceManager;
}

void ActorFactory::setAnimationLoader(AnimationLoader* p_AnimationLoader)
{
	m_AnimationLoader = p_AnimationLoader;
}

Actor::ptr ActorFactory::createActor(const tinyxml2::XMLElement* p_Data)
{
	return createActor(p_Data, getNextActorId());
}

Actor::ptr ActorFactory::createActor(const tinyxml2::XMLElement* p_Data, Actor::Id p_Id)
{
	Actor::ptr actor(new Actor(p_Id, m_EventManager));
	actor->initialize(p_Data);

	for (const tinyxml2::XMLElement* node = p_Data->FirstChildElement(); node; node = node->NextSiblingElement())
	{
		ActorComponent::ptr component(createComponent(node));
		if (component)
		{
			actor->addComponent(component);
			component->setOwner(actor.get());
		}
		else
		{
			return Actor::ptr();
		}
	}

	actor->postInit();

	return actor;
}

Actor::ptr ActorFactory::createBasicModel(const std::string& p_Model, Vector3 p_Position)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Model");
	printer.PushAttribute("Mesh", p_Model.c_str());
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

void addEdge(tinyxml2::XMLPrinter& p_Printer, Vector3 p_Position, Vector3 p_Halfsize)
{
	p_Printer.OpenElement("AABBPhysics");
	p_Printer.PushAttribute("Edge", true);
	pushVector(p_Printer, "Halfsize", p_Halfsize);
	pushVector(p_Printer, "OffsetPosition", p_Position);
	p_Printer.CloseElement();
}

Actor::ptr ActorFactory::createCollisionSphere(Vector3 p_Position, float p_Radius)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("SpherePhysics");
	printer.PushAttribute("Radius", p_Radius);
	pushVector(printer, "Position", p_Position);
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

Actor::ptr ActorFactory::createCheckPointActor(Vector3 p_Position, Vector3 p_Scale)
{
	Vector3 AABBScale = p_Scale;
	AABBScale.x *= 75.f;
	AABBScale.y *= 500.f;
	AABBScale.z *= 75.f;

	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Model");
	printer.PushAttribute("Mesh", "Checkpoint1");
	pushVector(printer, "Scale", p_Scale);
	printer.CloseElement();
	printer.OpenElement("AABBPhysics");
	printer.PushAttribute("CollisionResponse", false);
	pushVector(printer, "Halfsize", AABBScale);
	pushVector(printer, "OffsetPosition", Vector3(0.0f, AABBScale.y, 0.0f));
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

std::string ActorFactory::getPlayerActorDescription(Vector3 p_Position) const
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Model");
	printer.PushAttribute("Mesh", "WITCH");
	printer.CloseElement();
	printer.OpenElement("SpherePhysics");
	printer.PushAttribute("Immovable", false);
	printer.PushAttribute("Radius", 50.f);
	printer.PushAttribute("Mass", 68.f);
	pushVector(printer, "OffsetPosition", Vector3(0.f, 50.f, 0.f));
	printer.CloseElement();
	printer.OpenElement("Pulse");
	printer.PushAttribute("Length", 0.5f);
	printer.PushAttribute("Strength", 0.5f);
	printer.CloseElement();
	printer.OpenElement("Look");
	printer.CloseElement();
	printer.OpenElement("HumanAnimation");
	printer.PushAttribute("Animation", "WITCH");
	printer.CloseElement();
	printer.CloseElement();

	return printer.CStr();
}

Actor::ptr ActorFactory::createPlayerActor(Vector3 p_Position)
{
	tinyxml2::XMLDocument doc;
	doc.Parse(getPlayerActorDescription(p_Position).c_str());

	return createActor(doc.FirstChildElement("Object"));
}

Actor::ptr ActorFactory::createDirectionalLight(Vector3 p_Direction, Vector3 p_Color)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	printer.OpenElement("Light");
	printer.PushAttribute("Type", "Directional");
	pushVector(printer, "Direction", p_Direction);
	pushColor(printer, "Color", p_Color);
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

Actor::ptr ActorFactory::createSpotLight(Vector3 p_Position, Vector3 p_Direction, Vector2 p_MinMaxAngles, float p_Range, Vector3 p_Color)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Light");
	printer.PushAttribute("Type", "Spot");
	printer.PushAttribute("Range", p_Range);
	pushVector(printer, "Position", p_Position);
	pushVector(printer, "Direction", p_Direction);
	printer.OpenElement("Angles");
	printer.PushAttribute("min", p_MinMaxAngles.x);
	printer.PushAttribute("max", p_MinMaxAngles.y);
	printer.CloseElement();
	pushColor(printer, "Color", p_Color);
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

Actor::ptr ActorFactory::createPointLight(Vector3 p_Position, float p_Range, Vector3 p_Color)
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Light");
	printer.PushAttribute("Type", "Point");
	printer.PushAttribute("Range", p_Range);
	pushVector(printer, "Position", p_Position);
	pushColor(printer, "Color", p_Color);
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	Actor::ptr actor = createActor(doc.FirstChildElement("Object"));

	return actor;
}

Actor::ptr ActorFactory::createCheckPointArrow()
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	printer.OpenElement("Mesh");
	pushVector(printer, "Scale", Vector3(1.0f, 1.0f, 1.0f));
	pushVector(printer, "ColorTone", Vector3(1.0f, 1.0f, 1.0f));
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	return createActor(doc.FirstChildElement("Object"));
}

Actor::ptr ActorFactory::createParticles( Vector3 p_Position, const std::string& p_Effect )
{
	tinyxml2::XMLPrinter printer;
	printer.OpenElement("Object");
	pushVector(printer, p_Position);
	printer.OpenElement("Particle");
	printer.PushAttribute("Effect", p_Effect.c_str());
	printer.CloseElement();
	printer.CloseElement();

	tinyxml2::XMLDocument doc;
	doc.Parse(printer.CStr());

	return createActor(doc.FirstChildElement("Object"));
}

ActorComponent::ptr ActorFactory::createComponent(const tinyxml2::XMLElement* p_Data)
{
	std::string name(p_Data->Value());

	ActorComponent::ptr component;

	auto findIt = m_ComponentCreators.find(name);
	if (findIt != m_ComponentCreators.end())
	{
		componentCreatorFunc creator = findIt->second;
		component = creator();
	}
	else
	{
		throw CommonException("Could not find ActorComponent creator named '" + name + "'", __LINE__, __FILE__);
	}

	if (component)
	{
		component->initialize(p_Data);
	}

	return component;
}

unsigned int ActorFactory::getNextActorId()
{
	return ++m_LastActorId;
}

ActorComponent::ptr ActorFactory::createOBBComponent()
{
	OBB_Component* comp = new OBB_Component;
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createCollisionSphereComponent()
{
	CollisionSphereComponent* comp = new CollisionSphereComponent;
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createAABBComponent()
{
	AABB_Component* comp = new AABB_Component;
	comp->setPhysics(m_Physics);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createBoundingMeshComponent()
{
	BoundingMeshComponent* comp = new BoundingMeshComponent;
	comp->setPhysics(m_Physics);
	comp->setResourceManager(m_ResourceManager);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createModelComponent()
{
	ModelComponent* comp = new ModelComponent;
	comp->setId(++m_LastModelComponentId);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createMovementComponent()
{
	return ActorComponent::ptr(new MovementComponent);
}

ActorComponent::ptr ActorFactory::createCircleMovementComponent()
{
	return ActorComponent::ptr(new CircleMovementComponent);
}

ActorComponent::ptr ActorFactory::createPulseComponent()
{
	return ActorComponent::ptr(new PulseComponent);
}

ActorComponent::ptr ActorFactory::createLightComponent()
{
	LightComponent* comp = new LightComponent;
	comp->setId(++m_LastLightComponentId);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createParticleComponent()
{
	ParticleComponent* comp = new ParticleComponent;
	comp->setId(++m_LastParticleComponentId);

	return ActorComponent::ptr(comp);
}

ActorComponent::ptr ActorFactory::createLookComponent()
{
	return ActorComponent::ptr(new LookComponent);
}

ActorComponent::ptr ActorFactory::createHumanAnimationComponent()
{
	HumanAnimationComponent* comp = new HumanAnimationComponent;
	comp->setResourceManager(m_ResourceManager);
	comp->setAnimationLoader(m_AnimationLoader);

	return ActorComponent::ptr(comp);
}
