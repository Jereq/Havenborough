#pragma once

#include <map>
#include <string>

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
	std::map<std::string, std::string> m_ObjectDescriptions;

public:
	ObjectManager(ActorFactory::ptr p_ActorFactory, EventManager* p_EventManager, ResourceManager* p_ResourceManager);
	~ObjectManager();

	void update(float p_DeltaTime);

	void loadLevel(const std::string& p_Filename);
	void addObject(const std::string& p_ObjectName, const Vector3& p_Position);
	void registerObjectDescription(const std::string& p_ObjectName, const std::string& p_Description);

	Actor::ptr getActor(Actor::Id p_Id);

public:
signals:
	void meshCreated(std::string p_MeshName, int p_ActorId);
	void lightCreated(std::string p_LightName, int p_ActorId);
	void particleCreated(std::string p_ParticleName, int p_ActorId);
};
