#include "Table.h"

#include "TableItem.h"

Table::Table(QWidget* parent)
	: QTableWidget(parent)
{
}

void Table::addObject(std::string p_ObjectName)
{
	if(m_TableIndex.x > 17)
	{
		insertRow(rowCount());
		m_TableIndex.x = 0;
		m_TableIndex.y++;
	}
		
	if(m_TableIndex.y == 0 && m_TableIndex.x <= 17)
		insertColumn(columnCount());

	setItem(m_TableIndex.y, m_TableIndex.x, new TableItem(p_ObjectName));

	m_TableIndex.x++;

	resizeColumnsToContents();
	resizeRowsToContents();
}
