#include "XMLLevel.h"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <ActorFactory.h>
#include <ActorList.h>

#include "Tree.h"
#include "TreeFilter.h"
#include "TreeItem.h"

XMLLevel::XMLLevel(ActorList& p_ActorList, Tree* p_ObjectTree)
	: m_ActorList(p_ActorList),
	m_ObjectTree(p_ObjectTree)
{
}

void XMLLevel::save(std::ostream& p_Out) const
{
	std::vector<const TreeItem*> items;
	std::vector<const TreeFilter*> filters;
	for (int i = 0; i < m_ObjectTree->topLevelItemCount(); ++i)
	{
		retreiveNodes(m_ObjectTree->topLevelItem(i), items, filters);
	}

	using namespace tinyxml2;

	XMLPrinter printer;
	printer.OpenElement("Level");

	for (auto filter : filters)
	{
		printFilter(printer, filter);
	}

	for (auto item : items)
	{
		printItem(printer, item);
	}

	printer.CloseElement();

	p_Out.write(printer.CStr(), printer.CStrSize() - 1);
}

void XMLLevel::load(std::istream& p_In, ActorFactory::ptr p_Factory)
{
	using namespace tinyxml2;

	auto start = p_In.tellg();
	p_In.seekg(0, std::ios_base::end);
	auto end = p_In.tellg();
	p_In.seekg(start);

	size_t size = end - start;
	std::vector<char> buffer(size);
	p_In.read(buffer.data(), size);

	XMLDocument doc;
	if (XMLError err = doc.Parse(buffer.data(), size))
	{
		throw std::exception("Failed to parse xml data");
	}

	m_ObjectTree->clearTree();
	m_ActorList = ActorList();

	XMLElement* root = doc.FirstChildElement("Level");
	if (!root)
	{
		throw std::exception("Invalid xml structure: missing Level root element");
	}

	struct TmpFilter
	{
		boost::uuids::uuid parent;
		std::string name;
	};

	boost::uuids::string_generator fromString;
	std::map<boost::uuids::uuid, TmpFilter> tmpFilters;
	for (XMLElement* filterElem = root->FirstChildElement("Filter"); filterElem; filterElem = filterElem->NextSiblingElement("Filter"))
	{
		const char* name = filterElem->Attribute("Name");
		const char* id = filterElem->Attribute("UUID");
		if (!name || !id)
		{
			continue;
		}

		TmpFilter filter;
		filter.name = name;
		if (const char* parent = filterElem->Attribute("Parent"))
		{
			filter.parent = fromString(parent);
		}
		else
		{
			filter.parent = boost::uuids::nil_uuid();
		}

		tmpFilters[fromString(id)] = filter;
	}

	std::map<boost::uuids::uuid, TreeFilter*> createdFilters;

	while (!tmpFilters.empty())
	{
		bool foundFilter = false;
		std::pair<boost::uuids::uuid, TmpFilter> toRemove;
		for (auto& tFilter : tmpFilters)
		{
			if (tFilter.second.parent.is_nil() || createdFilters.count(tFilter.second.parent))
			{
				toRemove = tFilter;
				foundFilter = true;
				break;
			}
		}

		if (!foundFilter)
		{
			throw std::exception("Invalid filter structure");
		}

		tmpFilters.erase(toRemove.first);
		TreeFilter* newFilter = new TreeFilter(toRemove.second.name, toRemove.first);
		if (toRemove.second.parent.is_nil())
		{
			m_ObjectTree->addTopLevelItem(newFilter);
		}
		else
		{
			TreeFilter* parent = createdFilters[toRemove.second.parent];
			parent->addChild(newFilter);
		}
		createdFilters[toRemove.first] = newFilter;
	}

	for (XMLElement* actorElem = root->FirstChildElement("Actor"); actorElem; actorElem = actorElem->NextSiblingElement("Actor"))
	{
		const char* name = actorElem->Attribute("Name");
		if (!name)
		{
			throw std::exception("Actor missing name");
		}

		XMLElement* desc = actorElem->FirstChildElement("Object");
		if (!desc)
		{
			throw std::exception("Actor missing description");
		}

		Actor::ptr actor = p_Factory->createActor(desc);
		m_ActorList.addActor(actor);

		TreeItem* item = new TreeItem(name, actor->getId(), TreeItem::TreeItemType::MODEL);
		if (const char* parentId = actorElem->Attribute("Filter"))
		{
			boost::uuids::uuid parentUUID = fromString(parentId);
			auto parentIter = createdFilters.find(parentUUID);
			if (parentIter == createdFilters.end())
			{
				throw std::exception("Invalid parent for actor");
			}

			parentIter->second->addChild(item);
		}
		else
		{
			m_ObjectTree->addTopLevelItem(item);
		}
	}
}

void XMLLevel::retreiveNodes(const QTreeWidgetItem* p_CurrentNode, std::vector<const TreeItem*>& p_ItemsOut, std::vector<const TreeFilter*>& p_FilterOut) const
{
	const TreeFilter* filter = dynamic_cast<const TreeFilter*>(p_CurrentNode);
	if (filter)
	{
		p_FilterOut.push_back(filter);
		for (int i = 0; i < filter->childCount(); ++i)
		{
			retreiveNodes(filter->child(i), p_ItemsOut, p_FilterOut);
		}
		return;
	}

	const TreeItem* item = dynamic_cast<const TreeItem*>(p_CurrentNode);
	if (item)
	{
		p_ItemsOut.push_back(item);
	}
}

void XMLLevel::printItem(tinyxml2::XMLPrinter& p_Printer, const TreeItem* p_Item) const
{
	p_Printer.OpenElement("Actor");
	p_Printer.PushAttribute("Name", p_Item->text(0).toUtf8().data());
	
	const TreeFilter* parentFilter = dynamic_cast<const TreeFilter*>(p_Item->parent());
	if (parentFilter)
	{
		const boost::uuids::uuid& parentId = parentFilter->getID();
		p_Printer.PushAttribute("Filter", boost::lexical_cast<std::string>(parentId).c_str());
	}

	Actor::ptr actor = m_ActorList.findActor(p_Item->getActorId());
	actor->serialize(p_Printer);

	p_Printer.CloseElement();
}

void XMLLevel::printFilter(tinyxml2::XMLPrinter& p_Printer, const TreeFilter* p_Filter)
{
	p_Printer.OpenElement("Filter");
	p_Printer.PushAttribute("Name", p_Filter->text(0).toUtf8().data());
	const boost::uuids::uuid& id = p_Filter->getID();
	p_Printer.PushAttribute("UUID", boost::lexical_cast<std::string>(id).c_str());

	TreeFilter* parent = dynamic_cast<TreeFilter*>(p_Filter->parent());
	if (parent)
	{
		const boost::uuids::uuid& parentId = parent->getID();
		p_Printer.PushAttribute("Parent", boost::lexical_cast<std::string>(parentId).c_str());
	}
	p_Printer.CloseElement();
}
