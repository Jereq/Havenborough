#include "TreeFilter.h"

#include <boost/uuid/random_generator.hpp>

TreeFilter::TreeFilter()
{
}

TreeFilter::TreeFilter(std::string p_FilterName)
	: m_Id(boost::uuids::random_generator()())
{
	m_Filterame = p_FilterName;

    setText(0, QString::fromStdString(m_Filterame));
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | 
		Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setIcon(0, QIcon(":/Icons/Assets/Filter.png"));
	setToolTip(0, "Filter: " + QString::fromStdString(m_Filterame));
}

TreeFilter::TreeFilter(std::string p_FilterName, const boost::uuids::uuid& p_Id)
	: m_Id(p_Id)
{
	m_Filterame = p_FilterName;

    setText(0, QString::fromStdString(m_Filterame));
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable |
		Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setIcon(0, QIcon(":/Icons/Assets/Filter.png"));
	setToolTip(0, "Filter: " + QString::fromStdString(m_Filterame));
}

const boost::uuids::uuid& TreeFilter::getID() const
{
	return m_Id;
}

void TreeFilter::setName(std::string p_ObjectName)
{
	m_Filterame = p_ObjectName;

    setText(0, QString::fromStdString(m_Filterame));
	setToolTip(0, "Filter: " + QString::fromStdString(m_Filterame));
}
