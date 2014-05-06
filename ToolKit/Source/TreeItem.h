#ifndef TREEITEM_H
#define TREEITEM_H

#include <QTreeWidgetItem>

class TreeItem : public QTreeWidgetItem
{
  public:
    explicit TreeItem();
    TreeItem(std::string p_ItemName, int p_ActorId);

	int getActorId() const;

  private:
	  std::string m_ItemName;
	  int m_ActorId;

};

#endif // TREEITEM_H
