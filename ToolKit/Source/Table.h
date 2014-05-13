#ifndef TABLE_H
#define TABLE_H

#include <QTableWidget>

class Table : public QTableWidget
{
	Q_OBJECT

private:
	struct intVec2
	{
		unsigned int x;
		unsigned int y;

		intVec2()
		{
			x = y = 0;
		}
	};
	intVec2 m_TableIndex;

public:
    Table(QWidget* parent);


public slots:
	void addObject(std::string p_ObjectName);

signals:

protected:
    QStyleOptionViewItem viewOptions() const
    {
        QStyleOptionViewItem option = QTableWidget::viewOptions();
        option.decorationAlignment = Qt::AlignHCenter | Qt::AlignCenter;
        option.decorationPosition = QStyleOptionViewItem::Top;
        option.decorationSize = QSize(200, 200);
		
        return option;
    }
};

#endif // TABLE_H
