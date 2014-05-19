#pragma once

#include <map>
#include <memory>
#include <string>

#include <QObject>

#include <tinyxml2\tinyxml2.h>

#include <ActorList.h>

class ActorFactory;
class EventManager;
class ResourceManager;
class Tree;
class QProgressDialog;

class ObjectManager : public QObject
{
	Q_OBJECT

private:
	std::shared_ptr<ActorFactory> m_ActorFactory;
	ActorList m_ActorList;
	EventManager* m_EventManager;
	ResourceManager* m_ResourceManager;
	std::map<std::string, std::unique_ptr<tinyxml2::XMLDocument>> m_ObjectDescriptions;
	Tree* m_ObjectTree;

public:
	ObjectManager(std::shared_ptr<ActorFactory> p_ActorFactory, EventManager* p_EventManager, ResourceManager* p_ResourceManager, Tree* p_ObjectTree);
	~ObjectManager();

	void update(float p_DeltaTime);

	void loadLevel(const std::string& p_Filename);
	void saveLevel(const std::string& p_Filename);
	void addObject(const std::string& p_ObjectName, const Vector3& p_Position);
	void registerObjectDescription(const std::string& p_ObjectName, const tinyxml2::XMLNode* p_Description);
	void loadDescriptionsFromFolder(const std::string& p_Path);

	Actor::ptr getActor(Actor::Id p_Id);
	Actor::ptr getActorFromBodyHandle(BodyHandle p_BodyHandle);

public:
signals:
	void actorAdded(std::string p_ObjectType, Actor::ptr p_Actor);
	void objectTypeCreated(std::string p_ObjectName);

private slots:
	void actorRemoved(int actorID);
};
