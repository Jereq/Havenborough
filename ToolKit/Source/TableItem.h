#ifndef TABLEITEM_H
#define TABLEITEM_H

#include <QTableWidgetItem>

class TableItem : public QTableWidgetItem
{
  public:
    explicit TableItem();
    TableItem(std::string p_ItemName);

  private:

};

#endif // TABLEITEM_H
