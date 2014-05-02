#pragma once

#include <ActorFactory.h>
#include <EventManager.h>
#include <ResourceManager.h>

class ObjectManager
{
private:
	ActorFactory::ptr m_ActorFactory;
	ActorList m_ActorList;
	EventManager* m_EventManager;
	ResourceManager* m_ResourceManager;

public:
	ObjectManager(ActorFactory::ptr p_ActorFactory, EventManager* p_EventManager, ResourceManager* p_ResourceManager);
	~ObjectManager();

	void update(float p_DeltaTime);

	void loadLevel(const std::string& p_Filename);
};
