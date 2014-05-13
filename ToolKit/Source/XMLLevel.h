#pragma once

#include <iostream>
#include <memory>
#include <vector>

#include <tinyxml2/tinyxml2.h>

class ActorFactory;
class ActorList;
class Tree;
class TreeFilter;
class TreeItem;
class QTreeWidgetItem;

class XMLLevel
{
private:
	ActorList& m_ActorList;
	Tree* m_ObjectTree;

public:
	XMLLevel(ActorList& p_ActorList, Tree* p_ObjectTree);
	void save(std::ostream& p_Out) const;
	void load(std::istream& p_In, std::shared_ptr<ActorFactory> p_Factory);

private:
	void retreiveNodes(const QTreeWidgetItem* p_CurrentNode, std::vector<const TreeItem*>& p_ItemsOut, std::vector<const TreeFilter*>& p_FilterOut) const;
	void printItem(tinyxml2::XMLPrinter& p_Printer, const TreeItem* p_Item) const;
	static void printFilter(tinyxml2::XMLPrinter& p_Printer, const TreeFilter* p_Filter);
};
