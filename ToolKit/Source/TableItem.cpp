#include "TableItem.h"

TableItem::TableItem()
{
}

TableItem::TableItem(std::string p_ItemName)
{
	if(p_ItemName.length() > 8)
		p_ItemName.insert(8, "-\n");

    setText(QString::fromStdString(p_ItemName));
    setTextAlignment(Qt::AlignBottom | Qt::AlignCenter);
    setIcon(QIcon(":/Icons/Assets/object.png"));
}
