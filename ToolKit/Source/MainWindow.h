#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
    void on_m_ObjectTreeAddButton_clicked();

    void on_m_ObjectTreeRemoveButton_clicked();

    void on_actionObject_Tree_triggered();

    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
