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
	addTopLevelItem(new TreeItem(p_MeshName + "_" + std::to_string(m_ObjectCount[p_MeshName]), p_ActorId, p_Type));
	++m_ObjectCount[p_MeshName];
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
            {
                currItemParent->addChild(newFilter);
            }
            else
            {
                insertTopLevelItem(0, newFilter);
                setCurrentItem(newFilter);
            }
        }
    }
    else
    {
        insertTopLevelItem(0, newFilter);
        setCurrentItem(newFilter);
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
			{
				TreeItem* treeItem = (TreeItem*)currChild;

				if(treeItem)
				{
					int id = treeItem->getActorId();

					emit removeActor(id);
				}

                delete currChild;
			}
        }

		TreeItem* treeItem = (TreeItem*)currItem;

		if(treeItem)
		{
			int id = treeItem->getActorId();

			emit removeActor(id);
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
			QTreeWidgetItem *currChild = currItem->child(i);
			TreeItem *item = dynamic_cast<TreeItem*>(currChild);

			if(item && item->getActorId() == p_ActorId)
			{
				setCurrentIndex(indexFromItem(currChild));
				break;
			}
			else if(currChild->childCount() > 0)
				selectItemTraverse(currChild, p_ActorId);
        }
    }
}
