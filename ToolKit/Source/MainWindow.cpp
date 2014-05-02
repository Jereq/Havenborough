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

    //Layout setup
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    //Test Objects & ObjectTree Test code
	TreeItem *item1 = new TreeItem("Object1");
    TreeItem *item2 = new TreeItem("Object2");
    TreeItem *item3 = new TreeItem("Object3");

    ui->m_ObjectTree->addTopLevelItem(item1);
    ui->m_ObjectTree->addTopLevelItem(item2);
    ui->m_ObjectTree->addTopLevelItem(item3);

    TreeFilter *filter1 = new TreeFilter("Filter1");
    ui->m_ObjectTree->addTopLevelItem(filter1);

	m_Timer.setInterval(1000 / 60);
	m_Timer.setSingleShot(false);
	QObject::connect(&m_Timer, SIGNAL(timeout()), this, SLOT(idle()));
	m_Timer.start();
    //Tablewidget test code
    QIcon icon(":/Icons/Assets/Filter.png");
    QTableWidgetItem *item = new QTableWidgetItem();
    item->setIcon(icon);
    item->setText("Object");
    item->setTextAlignment(Qt::AlignBottom | Qt::AlignCenter);

    ui->m_ObjectTable->setItem(0,0,item);
    ui->m_ObjectTable->resizeColumnsToContents();
    ui->m_ObjectTable->resizeRowsToContents();
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

void MainWindow::idle()
{
	ui->m_RenderWidget->updateStep(m_Timer.interval() / 1000.f);
	ui->m_RenderWidget->update();
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
