#include "TreeItem.h"

TreeItem::TreeItem()
{
}

TreeItem::TreeItem(std::string p_ItemName)
{
    setText(0, QString::fromStdString(p_ItemName));
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setIcon(0, QIcon(":/Icons/Assets/object.png"));
}
