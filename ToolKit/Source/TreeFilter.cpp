#include "TreeFilter.h"

TreeFilter::TreeFilter(QWidget *parent)
{
}

TreeFilter::TreeFilter(std::string p_FilterName)
{
    setText(0, QString::fromStdString(p_FilterName));
    setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    //Debug coloring
    setBackground(0,*(new QBrush(Qt::green,Qt::SolidPattern)));
}
