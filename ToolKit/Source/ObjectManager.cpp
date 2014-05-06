#include "ObjectManager.h"

#include <iostream>

#include "Level.h"

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
}
