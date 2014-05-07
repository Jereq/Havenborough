#include "TreeFilter.h"

TreeFilter::TreeFilter()
{
}

TreeFilter::TreeFilter(std::string p_FilterName)
{
	m_Filterame = p_FilterName;

    setText(0, QString::fromStdString(m_Filterame));
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setIcon(0, QIcon(":/Icons/Assets/Filter.png"));
	setToolTip(0, "Filter: " + QString::fromStdString(m_Filterame));
}


void TreeFilter::setName(std::string p_ObjectName)
{
	m_Filterame = p_ObjectName;

    setText(0, QString::fromStdString(m_Filterame));
	setToolTip(0, "Filter: " + QString::fromStdString(m_Filterame));
}