#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "TreeItem.h"
#include "TreeFilter.h"

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

	m_Timer.setInterval(1000 / 60);
	m_Timer.setSingleShot(false);
	QObject::connect(&m_Timer, SIGNAL(timeout()), this, SLOT(idle()));
	m_Timer.start();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_m_ObjectTreeAddButton_clicked()
{
    TreeFilter *newFilter = new TreeFilter("NewFilter");

	QTreeWidgetItem * currItem = ui->m_ObjectTree->currentItem()->parent();

	if(currItem)
		currItem->addChild(newFilter);
	else
		ui->m_ObjectTree->addTopLevelItem(newFilter);
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
