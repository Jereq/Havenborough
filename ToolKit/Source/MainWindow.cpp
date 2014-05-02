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

    //Timer
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

    QObject::connect(ui->m_RenderWidget, SIGNAL(CameraPositionChanged(Vector3)), this, SLOT(splitCameraPosition(Vector3)));
    QObject::connect(ui->m_CameraPositionXBox, SIGNAL(editingFinished()), this, SLOT(setCameraPosition()));
    QObject::connect(ui->m_CameraPositionYBox, SIGNAL(editingFinished()), this, SLOT(setCameraPosition()));
    QObject::connect(ui->m_CameraPositionZBox, SIGNAL(editingFinished()), this, SLOT(setCameraPosition()));
    QObject::connect(this, SIGNAL(setCameraPositionSignal(Vector3)), ui->m_RenderWidget, SLOT(CameraPositionSet(Vector3)));
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
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();

    removeChild(currItem);
}

void MainWindow::removeChild(QTreeWidgetItem* currItem)
{
    for (int i = 0; i < currItem->childCount(); i++)
    {
        QTreeWidgetItem *currChild = currItem->takeChild(i);
        if (currChild->childCount() != 0)
        {
            removeChild(currChild);
        }
        else
            delete currChild;
    }

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

void MainWindow::splitCameraPosition(Vector3 p_cameraPosition)
{
    ui->m_CameraPositionXBox->setValue(p_cameraPosition.x);
    ui->m_CameraPositionYBox->setValue(p_cameraPosition.y);
    ui->m_CameraPositionZBox->setValue(p_cameraPosition.z);
}

void MainWindow::setCameraPosition()
{
    emit setCameraPositionSignal(Vector3(ui->m_CameraPositionXBox->value(), ui->m_CameraPositionYBox->value(), ui->m_CameraPositionZBox->value()));
}
