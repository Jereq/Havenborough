#include "Tree.h"

#include "TreeFilter.h"
#include "TreeItem.h"

Tree::Tree(QWidget* parent) 
	: QTreeWidget(parent)
{
	QObject::connect(this, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(changeObjectName(QTreeWidgetItem *, int)));
}

void Tree::clearTree()
{
	clear();
	m_ObjectCount.clear();
}

void Tree::selectItem(int p_ActorId)
{


	QTreeWidgetItem *previous = currentItem();
	
	QTreeWidgetItem *currItem;
              	int tempId = p_ActorId;
	for(int i = 0; i < topLevelItemCount(); i++)
	{
		currItem = topLevelItem(i);
		TreeItem *item = dynamic_cast<TreeItem*>(currItem);
		if(item && item->getActorId() == tempId)
		{
			setCurrentIndex(indexFromItem(currItem));
			break;
		}
		else if(currItem->childCount() > 0)
			selectItemTraverse(currItem, tempId);
	}
}

void Tree::removeItem()
{
	QTreeWidgetItem *currItem = currentItem();

    removeChild(currItem);
}

void Tree::objectCreated(std::string p_MeshName, int p_ActorId, int p_Type)
{
	if(m_ObjectCount.count(p_MeshName) > 0)
		m_ObjectCount.at(p_MeshName)++;
	else
	{
		m_ObjectCount.insert(std::pair<std::string, int>(p_MeshName, 0));

		emit addTableObject(p_MeshName);
	}

	addTopLevelItem(new TreeItem(p_MeshName + "_" + std::to_string(m_ObjectCount.at(p_MeshName)), p_ActorId, p_Type));
}

void Tree::changeObjectName(QTreeWidgetItem *p_Item, int p_Column)
{
	TreeFilter *filter = dynamic_cast<TreeFilter*>(p_Item);
	TreeItem *item = dynamic_cast<TreeItem*>(p_Item);

	if(filter)
		filter->setName(p_Item->text(0).toStdString());

	if(item)
		item->setName(p_Item->text(0).toStdString());
}

void Tree::addFilter()
{
    TreeFilter *newFilter = new TreeFilter("NewFilter");

    QTreeWidgetItem *currItem = currentItem();
    if(currItem)
    {
        QTreeWidgetItem *currItemParent = currItem->parent();
        TreeFilter *cFilter = dynamic_cast<TreeFilter*>(currItem);

        if(cFilter)
        {
            currItem->addChild(newFilter);
        }
        else
        {
            if(currItemParent)
                currItemParent->addChild(newFilter);
            else
                addTopLevelItem(newFilter);
        }
    }
    else
    {
        addTopLevelItem(newFilter);
    }
}

void Tree::removeChild(QTreeWidgetItem* currItem)
{
    if(currItem && currItem->isSelected())
    {
        for (int i = 0; i < currItem->childCount(); i++)
        {
            QTreeWidgetItem *currChild = currItem->takeChild(i);
            if (currChild->childCount() != 0)
            {
                removeChild(currChild);
            }
            else
                delete currChild;
        }

        delete currItem;
    }
}

void Tree::selectItemTraverse(QTreeWidgetItem* currItem, int& p_ActorId)
{
	if(currItem)
    {
        for (int i = 0; i < currItem->childCount(); i++)
        {
			QTreeWidgetItem *currChild = currItem->takeChild(i);
			TreeItem *item = dynamic_cast<TreeItem*>(currChild);

			if(item && item->getActorId() == p_ActorId)
			{
				setCurrentIndex(indexFromItem(currItem));
				break;
			}
			else if(currItem->childCount() > 0)
				selectItemTraverse(currItem, p_ActorId);
        }
    }
}
