#include "ObjectManager.h"

#include <iostream>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>

#include <Components.h>
#include <Level.h>

#include "Tree.h"
#include "XMLLevel.h"

ObjectManager::ObjectManager(ActorFactory::ptr p_ActorFactory, EventManager* p_EventManager, ResourceManager* p_ResourceManager, Tree* p_ObjectTree)
	: m_ActorFactory(p_ActorFactory),
	m_EventManager(p_EventManager),
	m_ResourceManager(p_ResourceManager),
	m_ObjectTree(p_ObjectTree)
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
	std::ifstream istream(p_Filename, std::ifstream::binary);

	boost::filesystem::path filePath(p_Filename);
	if (filePath.extension() == ".xml")
	{
		XMLLevel levelLoader(m_ActorList, m_ObjectTree);
		levelLoader.load(istream, m_ActorFactory);
	}
	else
	{
		m_ObjectTree->clearTree();
		m_ActorList = ActorList();
		Level level(m_ResourceManager, m_ActorFactory.get(), m_EventManager);
		level.loadLevel(istream, ActorList::ptr(&m_ActorList, [](void const*){}));

		for(const auto &actor : m_ActorList)
		{
			emit actorAdded("", actor.second);
		}
	}
}

void ObjectManager::saveLevel(const std::string& p_Filename)
{
	XMLLevel levelSaver(m_ActorList, m_ObjectTree);
	std::ofstream outStream(p_Filename, std::ios_base::binary);
	levelSaver.save(outStream);
}

Actor::ptr ObjectManager::getActor(Actor::Id p_Id)
{
	return m_ActorList.findActor(p_Id);
}

Actor::ptr ObjectManager::getActorFromBodyHandle(BodyHandle p_BodyHandle)
{
	for(auto a = m_ActorList.begin(); a != m_ActorList.end(); a++)
	{
		if(a->second->getBodyHandles().size() > 0)
		{
			if(a->second->getBodyHandles()[0] == p_BodyHandle)
				return a->second;
		}
	}

	return nullptr;
}

void ObjectManager::addObject(const std::string& p_ObjectName, const Vector3& p_Position)
{
	tinyxml2::XMLDocument doc;
	if(doc.LoadFile(p_ObjectName.c_str()))
	{
		return;
	}

	tinyxml2::XMLElement* root = doc.FirstChildElement("ObjectDescription");
	if (!root)
	{
		return;
	}

	const char* objectName = root->Attribute("Name");
	if(!objectName)
	{
		return;
	}

	tinyxml2::XMLElement* objectDesc = root->FirstChildElement("Object");
	if (!objectDesc)
	{
		return;
	}

	Actor::ptr actor = m_ActorFactory->createActor(objectDesc);
	actor->setPosition(p_Position);

	m_ActorList.addActor(actor);
	emit actorAdded(objectName, actor);
}

static void deepClone(tinyxml2::XMLNode* p_NewNode, const tinyxml2::XMLNode* p_SrcNode)
{
	for (const tinyxml2::XMLNode* srcChild = p_SrcNode->FirstChild(); srcChild; srcChild = srcChild->NextSibling())
	{
		tinyxml2::XMLNode* clonedChild = srcChild->ShallowClone(p_NewNode->GetDocument());
		deepClone(clonedChild, srcChild);
		p_NewNode->InsertEndChild(clonedChild);
	}
}

void ObjectManager::registerObjectDescription(const std::string& p_ObjectName, const tinyxml2::XMLNode* p_Description)
{
	auto& doc = m_ObjectDescriptions[p_ObjectName];
	doc.reset(new tinyxml2::XMLDocument);
	deepClone(doc.get(), p_Description);
	objectTypeCreated(p_ObjectName);
}

void ObjectManager::loadDescriptionsFromFolder(const std::string& p_Path)
{
	using namespace boost::filesystem;

	path folder(p_Path);
	
	if (!exists(folder))
	{
		return;
	}

	for (recursive_directory_iterator iter(folder); iter != recursive_directory_iterator(); ++iter)
	{
		tinyxml2::XMLDocument doc;
		if (doc.LoadFile(iter->path().string().c_str()))
		{
			continue;
		}

		tinyxml2::XMLElement* root = doc.FirstChildElement("ObjectDescription");
		if (!root)
		{
			continue;
		}

		const char* descName = root->Attribute("Name");
		if (!descName)
		{
			continue;
		}

		registerObjectDescription(descName, root);
	}
}

void ObjectManager::actorRemoved(int actorID)
{
	m_ActorList.removeActor(actorID);
}
