#ifndef TREE_H
#define TREE_H

#include <QTreeWidgetItem>
#include <map>

class TreeItem;
class QMouseEvent;

class Tree : public QTreeWidget
{
	Q_OBJECT

private:
	std::map<std::string, int> m_ObjectCount;

public:
    Tree(QWidget* parent);

	void clearTree();
	
	void selectItem(int p_ActorId);
	QList<TreeItem*> getAllTreeItems(void);
public slots:
    void addFilter();
	void removeItem();
	void objectCreated(std::string p_ObjectName, int p_ActorId, int p_Type);

	void changeObjectName(QTreeWidgetItem *p_Item, int p_Column);

signals:
	void removeActor(int id);
	void deselectAll();

private:
    void removeChild(QTreeWidgetItem* currItem);
	void selectItemTraverse(QTreeWidgetItem* currItem, int &p_ActorId);
	void collectTreeItems(QTreeWidgetItem *currentItem, QList<TreeItem*> *itemList);

	virtual void mousePressEvent(QMouseEvent *mouseEvent) override;
};

#endif // TREE_H
