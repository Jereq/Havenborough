#pragma once

#include <ActorFactory.h>
#include <EventManager.h>
#include <ResourceManager.h>

#include <QObject>

#include "Components.h"


class ObjectManager : public QObject
{
	Q_OBJECT

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

	Actor::ptr getActor(Actor::Id p_Id);

public:
signals:
	void meshCreated(std::string p_MeshName, int p_ActorId);
	void lightCreated(std::string p_LightName, int p_ActorId);
	void particleCreated(std::string p_ParticleName, int p_ActorId);
};
