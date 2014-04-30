#include "TreeItem.h"

TreeItem::TreeItem()
{
}

TreeItem::TreeItem(std::string p_ItemName)
{
    setText(0, QString::fromStdString(p_ItemName));
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    //Debug coloring
    setBackground(0,*(new QBrush(Qt::red,Qt::SolidPattern)));
}
