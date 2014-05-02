#ifndef TREEITEM_H
#define TREEITEM_H

#include <QTreeWidgetItem>

class TreeItem : public QTreeWidgetItem
{
  public:
    explicit TreeItem();
    TreeItem(std::string p_ItemName);

  private:

};

#endif // TREEITEM_H
