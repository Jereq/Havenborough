#include "TreeFilter.h"

TreeFilter::TreeFilter()
{
}

TreeFilter::TreeFilter(std::string p_FilterName)
{
    setText(0, QString::fromStdString(p_FilterName));
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    setIcon(0, QIcon(":/Icons/Assets/Filter.png"));
}
