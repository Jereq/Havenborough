#include "XMLLevel.h"

#include <boost/lexical_cast.hpp>
#include <boost/uuid/nil_generator.hpp>
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
