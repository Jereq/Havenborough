#ifndef TREE_H
#define TREE_H

#include <QTreeWidgetItem>
#include <map>

class Tree : public QTreeWidget
{
	Q_OBJECT

private:
	std::map<std::string, int> m_ObjectCount;

public:
    Tree(QWidget* parent);

	void clearTree();

	void selectItem(int p_ActorId);

public slots:
    void addFilter();
	void removeItem();
	void objectCreated(std::string p_ObjectName, int p_ActorId, int p_Type);

	void changeObjectName(QTreeWidgetItem *p_Item, int p_Column);

signals:
	void addTableObject(std::string p_ObjectName);

private:
    void removeChild(QTreeWidgetItem* currItem);
	void selectItemTraverse(QTreeWidgetItem* currItem, int &p_ActorId);
};

#endif // TREE_H
