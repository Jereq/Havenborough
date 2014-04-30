#ifndef EXTENDEDTABLEWIDGET_H
#define EXTENDEDTABLEWIDGET_H

#include <QTableWidget>

class ExtendedTableWidget : public QTableWidget
{
public:
    explicit ExtendedTableWidget(QWidget *parent = 0) : QTableWidget(parent){}
private:

protected:


    // QAbstractItemView interface
protected:
    QStyleOptionViewItem viewOptions() const
    {
        QStyleOptionViewItem option = QTableWidget::viewOptions();
        option.decorationAlignment = Qt::AlignHCenter | Qt::AlignCenter;
        option.decorationPosition = QStyleOptionViewItem::Top;

        return option;

    }
};

#endif // EXTENDEDTABLEWIDGET_H
