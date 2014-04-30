#ifndef TREEFILTER_H
#define TREEFILTER_H

#include <QTreeWidgetItem>

class TreeFilter : public QTreeWidgetItem
{
  public:
    explicit TreeFilter(QWidget* parent = 0);
    TreeFilter(std::string p_FilterName);

  private:

};

#endif // TREEFILTER_H
