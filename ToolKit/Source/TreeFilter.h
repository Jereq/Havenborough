#ifndef TREEFILTER_H
#define TREEFILTER_H

#include <boost/uuid/uuid.hpp>

#include <QTreeWidgetItem>

class TreeFilter : public QTreeWidgetItem
{
private:
	boost::uuids::uuid m_Id;
	  std::string m_Filterame;

  public:
    explicit TreeFilter();
    TreeFilter(std::string p_FilterName);
	TreeFilter(std::string p_FilterName, const boost::uuids::uuid& p_Id);

	const boost::uuids::uuid& getID() const;
	void setName(std::string p_ObjectName);

};

#endif // TREEFILTER_H
