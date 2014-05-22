#include "TreeItem.h"

TreeItem::TreeItem()
{
}

TreeItem::TreeItem(std::string p_ItemName, int p_ActorId, int p_Type)
{
	m_ItemName = p_ItemName;
	m_ActorId = p_ActorId;
    m_Type = (TreeItemType)p_Type;

    setText(0, QString::fromStdString(m_ItemName));
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable |
		Qt::ItemIsEnabled);
    setIcon(0, QIcon(":/Icons/Assets/object.png"));
	setToolTip(0, "Object: " + QString::fromStdString(m_ItemName));
}

int TreeItem::getActorId() const
{
	return m_ActorId;
}

void TreeItem::setName(std::string p_ObjectName)
{
	m_ItemName = p_ObjectName;

    setText(0, QString::fromStdString(m_ItemName));
	setToolTip(0, "Object: " + QString::fromStdString(m_ItemName));
}

TreeItem::TreeItemType TreeItem::getType()
{
    return m_Type;
}
