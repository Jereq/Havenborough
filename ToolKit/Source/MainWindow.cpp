#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "TreeItem.h"
#include "TreeFilter.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);


	TreeItem *item1 = new TreeItem("Object1");
    TreeItem *item2 = new TreeItem("Object2");
    TreeItem *item3 = new TreeItem("Object3");

    ui->m_ObjectTree->addTopLevelItem(item1);
    ui->m_ObjectTree->addTopLevelItem(item2);
    ui->m_ObjectTree->addTopLevelItem(item3);

    TreeFilter *filter1 = new TreeFilter("Filter1");

    ui->m_ObjectTree->addTopLevelItem(filter1);
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_m_ObjectTreeAddButton_clicked()
{
    TreeFilter *newFilter = new TreeFilter("NewFilter");

    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();
    QTreeWidgetItem *currItemParent = currItem->parent();

    TreeFilter *cFilter = dynamic_cast<TreeFilter*>(currItem);

    if(cFilter)
    {
        currItem->addChild(newFilter);
    }
    else
    {
        if(currItemParent)
            currItemParent->addChild(newFilter);
        else
            ui->m_ObjectTree->addTopLevelItem(newFilter);
    }
}

void MainWindow::on_m_ObjectTreeRemoveButton_clicked()
{
	QTreeWidgetItem * currItem = ui->m_ObjectTree->currentItem();
	delete currItem;
}

void MainWindow::on_actionObject_Tree_triggered()
{
    ui->m_ObjectDockableWidget->show();
}

void MainWindow::on_actionExit_triggered()
{
    exit(0);
}

void MainWindow::on_actionOpen_triggered()
{
    QString fullFilePath = QFileDialog::getOpenFileName(this, tr("Open Level"), "/home/ME", tr("Level Files (*.xml)"));
}

void MainWindow::on_actionSave_triggered()
{
    QFileDialog::getSaveFileName(this, tr("Save Level As..."), "/home/ME", tr("Level Files (*.xml"));
}
