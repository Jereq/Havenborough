#ifndef TREEFILTER_H
#define TREEFILTER_H

#include <QTreeWidgetItem>

class TreeFilter : public QTreeWidgetItem
{
  public:
    explicit TreeFilter();
    TreeFilter(std::string p_FilterName);

	void setName(std::string p_ObjectName);

  private:
	  std::string m_Filterame;
};

#endif // TREEFILTER_H
