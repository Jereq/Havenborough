#include "ObjectManager.h"

#include <iostream>

#include <Components.h>
#include <Level.h>

ObjectManager::ObjectManager(ActorFactory::ptr p_ActorFactory, EventManager* p_EventManager, ResourceManager* p_ResourceManager)
	: m_ActorFactory(p_ActorFactory),
	m_EventManager(p_EventManager),
	m_ResourceManager(p_ResourceManager)
{
}

ObjectManager::~ObjectManager()
{

}

void ObjectManager::update(float p_DeltaTime)
{
	m_ActorList.onUpdate(p_DeltaTime);
}

void ObjectManager::loadLevel(const std::string& p_Filename)
{
	m_ActorList = ActorList();

	Level level(m_ResourceManager, m_ActorFactory.get(), m_EventManager);
	std::ifstream istream(p_Filename, std::ifstream::binary);
	level.loadLevel(istream, ActorList::ptr(&m_ActorList, [](void const*){}));

	for(const auto &actor : m_ActorList)
	{
		std::weak_ptr<ModelComponent> model = actor.second.get()->getComponent<ModelComponent>(ModelInterface::m_ComponentId);
		std::shared_ptr<ModelComponent> smodel = model.lock();
		if(smodel)
		{
			emit meshCreated(smodel->getMeshName(), actor.first);
		}
		std::weak_ptr<LightComponent> lmodel = actor.second.get()->getComponent<LightComponent>(LightInterface::m_ComponentId);
		std::shared_ptr<LightComponent> slmodel = lmodel.lock();
		if(slmodel)
		{
			std::string lightype = "Unknown";
			switch(slmodel->getType())
			{
			case LightClass::Type::DIRECTIONAL: lightype = "Directional"; break;
			case LightClass::Type::SPOT: lightype = "Spot"; break;
			case LightClass::Type::POINT: lightype = "Point"; break;
			}

			emit lightCreated(lightype, actor.first);
		}
		std::weak_ptr<ParticleComponent> pmodel = actor.second.get()->getComponent<ParticleComponent>(ParticleInterface::m_ComponentId);
		std::shared_ptr<ParticleComponent> spmodel = pmodel.lock();
		if(spmodel)
		{
			emit particleCreated(spmodel->getEffectName(), actor.first);
		}
	}
}

Actor::ptr ObjectManager::getActor(Actor::Id p_Id)
{
	return m_ActorList.findActor(p_Id);
}

void ObjectManager::addObject(const std::string& p_ObjectName, const Vector3& p_Position)
{
	if (m_ActorList.begin() == m_ActorList.end())
	{
		m_ActorList.addActor(m_ActorFactory->createDirectionalLight(Vector3(0.1f, -0.8f, 0.2f), Vector3(1.f, 1.f, 1.f), 1.f));
	}

	auto description = m_ObjectDescriptions.find(p_ObjectName);
	if (description == m_ObjectDescriptions.end())
	{
		return;
	}

	tinyxml2::XMLDocument descDoc;
	descDoc.Parse(description->second.c_str());

	tinyxml2::XMLElement* root = descDoc.FirstChildElement("Object");
	if (!root)
	{
		return;
	}

	Actor::ptr actor = m_ActorFactory->createActor(root);
	actor->setPosition(p_Position);

	m_ActorList.addActor(actor);
}

void ObjectManager::registerObjectDescription(const std::string& p_ObjectName, const std::string& p_Description)
{
	m_ObjectDescriptions[p_ObjectName] = p_Description;
}
