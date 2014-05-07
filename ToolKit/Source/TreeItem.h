#ifndef TREEITEM_H
#define TREEITEM_H

#include <QTreeWidgetItem>

class TreeItem : public QTreeWidgetItem
{
public:
    enum TreeItemType{
        DIRECTIONALLIGHT = 0,
        POINTLIGHT,
        SPOTLIGHT,
        MODEL,
        PARTICLE
    };
    explicit TreeItem();
    TreeItem(std::string p_ItemName, int p_ActorId, int p_Type);

    int getActorId() const;
    void setName(std::string p_ObjectName);
    TreeItemType getType();
private:
    std::string m_ItemName;
    int m_ActorId;
    TreeItemType m_Type;
};

#endif // TREEITEM_H
