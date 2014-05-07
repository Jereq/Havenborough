#include "TreeItem.h"

TreeItem::TreeItem()
{
}

TreeItem::TreeItem(std::string p_ItemName, int p_ActorId)
{
	m_ItemName = p_ItemName;
	m_ActorId = p_ActorId;


    setText(0, QString::fromStdString(m_ItemName));
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setIcon(0, QIcon(":/Icons/Assets/object.png"));
}

int TreeItem::getActorId() const
{
	return m_ActorId;
}
