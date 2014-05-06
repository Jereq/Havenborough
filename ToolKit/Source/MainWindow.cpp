#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "TreeItem.h"
#include "TreeFilter.h"
#include "TableItem.h"

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

    //Hide Position Scale Rotation by default
    ui->PositionBox->hide();
    ui->ScaleBox->hide();
    ui->RotationBox->hide();

    //Signals and slots for connecting the camera to the camerabox values
    QObject::connect(ui->m_RenderWidget, SIGNAL(CameraPositionChanged(Vector3)), this, SLOT(splitCameraPosition(Vector3)));
    QObject::connect(ui->m_CameraPositionXBox, SIGNAL(editingFinished()), this, SLOT(setCameraPosition()));
    QObject::connect(ui->m_CameraPositionYBox, SIGNAL(editingFinished()), this, SLOT(setCameraPosition()));
    QObject::connect(ui->m_CameraPositionZBox, SIGNAL(editingFinished()), this, SLOT(setCameraPosition()));
    QObject::connect(this, SIGNAL(setCameraPositionSignal(Vector3)), ui->m_RenderWidget, SLOT(CameraPositionSet(Vector3)));
    QObject::connect(ui->m_RenderWidget, SIGNAL(meshCreated(std::string)), this, SLOT(on_meshCreated_triggered(std::string)));

    //Timer
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
	close();
}

void MainWindow::on_actionOpen_triggered()
{
	QString fullFilePath = QFileDialog::getOpenFileName(this, tr("Open Level"), "/home/ME", tr("Level Files (*.xml *.btxl)"));
	if (!fullFilePath.isNull())
	{
		m_ObjectCount.clear();
		ui->m_ObjectTree->clear();

		ui->m_RenderWidget->loadLevel(fullFilePath.toStdString());
	}
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

void MainWindow::on_actionProperties_triggered()
{
    ui->m_PropertiesDockableWidget->show();
}

void MainWindow::on_actionAdd_Object_triggered()
{
    ui->m_ObjectTableDockableWidget->show();
}

void MainWindow::on_m_ObjectTree_itemSelectionChanged()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();

    TreeFilter *cFilter = dynamic_cast<TreeFilter*>(currItem);

    if(cFilter)
    {
        ui->PositionBox->hide();
        ui->ScaleBox->hide();
        ui->RotationBox->hide();
    }
    else
    {
        ui->PositionBox->show();
        ui->ScaleBox->show();
        ui->RotationBox->show();
    }
}

void MainWindow::on_meshCreated_triggered(std::string p_MeshName)
{
	if(m_ObjectCount.count(p_MeshName) > 0)
		m_ObjectCount.at(p_MeshName)++;
	else
	{
		m_ObjectCount.insert(std::pair<std::string, int>(p_MeshName, 0));

        ui->m_ObjectTable->setItem(m_TableIndex.y, m_TableIndex.x, new TableItem(p_MeshName));
        
        m_TableIndex.x++;

		if(m_TableIndex.x > 17)
		{
			ui->m_ObjectTable->insertRow(ui->m_ObjectTable->rowCount());
			m_TableIndex.x = 0;
			m_TableIndex.y++;
		}
		
		if(m_TableIndex.y == 0 && m_TableIndex.x <= 17)
			ui->m_ObjectTable->insertColumn(ui->m_ObjectTable->columnCount());

		ui->m_ObjectTable->resizeColumnsToContents();
        ui->m_ObjectTable->resizeRowsToContents();
	}

	ui->m_ObjectTree->addTopLevelItem(new TreeItem(p_MeshName + "_" + std::to_string(m_ObjectCount.at(p_MeshName))));
}
