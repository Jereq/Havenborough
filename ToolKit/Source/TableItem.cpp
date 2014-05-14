#include "TableItem.h"

TableItem::TableItem()
{
}

TableItem::TableItem(std::string p_ItemName)
{
    setText(QString::fromStdString(p_ItemName));
    setTextAlignment(Qt::AlignBottom | Qt::AlignCenter);
    setIcon(QIcon(":/Icons/Assets/object.png"));
}
