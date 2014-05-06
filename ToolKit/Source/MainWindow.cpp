#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "TreeItem.h"
#include "TreeFilter.h"
#include "TableItem.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_DefaultObjectIcon(":/Icons/Assets/object.png")
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

	QObject::connect(ui->m_RenderWidget->getObjectManager(), SIGNAL(meshCreated(std::string, int)), this, SLOT(on_meshCreated_triggered(std::string, int)));
	QObject::connect(ui->m_RenderWidget->getObjectManager(), SIGNAL(lightCreated(std::string, int)), this, SLOT(on_lightCreated_triggered(std::string, int)));
	QObject::connect(ui->m_RenderWidget->getObjectManager(), SIGNAL(particleCreated(std::string, int)), this, SLOT(on_particleCreated_triggered(std::string, int)));

    QObject::connect(ui->m_ObjectScaleXBox, SIGNAL(editingFinished()), this, SLOT(setObjectScale()));
    QObject::connect(ui->m_ObjectScaleYBox, SIGNAL(editingFinished()), this, SLOT(setObjectScale()));
    QObject::connect(ui->m_ObjectScaleZBox, SIGNAL(editingFinished()), this, SLOT(setObjectScale()));

    QObject::connect(ui->m_ObjectRotationXBox, SIGNAL(editingFinished()), this, SLOT(setObjectRotation()));
    QObject::connect(ui->m_ObjectRotationYBox, SIGNAL(editingFinished()), this, SLOT(setObjectRotation()));
    QObject::connect(ui->m_ObjectRotationZBox, SIGNAL(editingFinished()), this, SLOT(setObjectRotation()));

    QObject::connect(ui->m_ObjectPositionXBox, SIGNAL(editingFinished()), this, SLOT(setObjectPosition()));
    QObject::connect(ui->m_ObjectPositionYBox, SIGNAL(editingFinished()), this, SLOT(setObjectPosition()));
    QObject::connect(ui->m_ObjectPositionZBox, SIGNAL(editingFinished()), this, SLOT(setObjectPosition()));

    // Nest dock widgets.
    tabifyDockWidget(ui->m_ParticleTreeDockableWidget, ui->m_LightTreeDockableWidget);
    tabifyDockWidget(ui->m_ParticleTreeDockableWidget, ui->m_ObjectDockableWidget);

    //Timer
	m_Timer.setInterval(1000 / 60);
	m_Timer.setSingleShot(false);
	QObject::connect(&m_Timer, SIGNAL(timeout()), this, SLOT(idle()));
	m_Timer.start();
	addSimpleObjectType("Barrel1");
	addSimpleObjectType("House1");
	addSimpleObjectType("Island1");
	addSimpleObjectType("Top1");
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::on_m_ObjectTreeAddButton_clicked()
{
	addFilter(ui->m_ObjectTree);
}

void MainWindow::on_m_ObjectTreeRemoveButton_clicked()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();

    removeChild(currItem);
}

void MainWindow::removeChild(QTreeWidgetItem* currItem)
{
	if(currItem && currItem->isSelected())
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
}

void MainWindow::addFilter(QTreeWidget* tree)
{
    TreeFilter *newFilter = new TreeFilter("NewFilter");

    QTreeWidgetItem *currItem = tree->currentItem();
	if(currItem)
	{
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
				tree->addTopLevelItem(newFilter);
		}
	}
	else
	{
		tree->addTopLevelItem(newFilter);
	}
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

void MainWindow::on_actionParticle_Tree_triggered()
{
    ui->m_ParticleTreeDockableWidget->show();
}

void MainWindow::on_actionLight_Tree_triggered()
{
    ui->m_LightTreeDockableWidget->show();
}

void MainWindow::on_m_ObjectTree_itemSelectionChanged()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();


	ui->PositionBox->hide();
    ui->ScaleBox->hide();
    ui->RotationBox->hide();

    if(currItem && currItem->isSelected())
	{
        TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);
        if(cItem)
        {
            ui->PositionBox->show();
            ui->ScaleBox->show();
            ui->RotationBox->show();

            Actor::ptr actor = ui->m_RenderWidget->getObjectManager()->getActor(cItem->getActorId());
            std::weak_ptr<ModelComponent> pmodel = actor->getComponent<ModelComponent>(ModelInterface::m_ComponentId);
            std::shared_ptr<ModelComponent> spmodel = pmodel.lock();

            Vector3 scale = spmodel->getScale();
            Vector3 position = spmodel->getPosition();
            Vector3 rotation = spmodel->getRotation();

            ui->m_ObjectScaleXBox->setValue(scale.x);
            ui->m_ObjectScaleYBox->setValue(scale.y);
            ui->m_ObjectScaleZBox->setValue(scale.z);

            ui->m_ObjectPositionXBox->setValue(position.x);
            ui->m_ObjectPositionYBox->setValue(position.y);
            ui->m_ObjectPositionZBox->setValue(position.z);

            ui->m_ObjectRotationXBox->setValue(rotation.x);
            ui->m_ObjectRotationYBox->setValue(rotation.y);
            ui->m_ObjectRotationZBox->setValue(rotation.z);
        }
	}
}

void MainWindow::on_meshCreated_triggered(std::string p_MeshName, int p_ActorId)
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

	ui->m_ObjectTree->addTopLevelItem(new TreeItem(p_MeshName + "_" + std::to_string(m_ObjectCount.at(p_MeshName)), p_ActorId));
}

void MainWindow::on_lightCreated_triggered(std::string p_MeshName, int p_ActorId)
{
	if(m_ObjectCount.count(p_MeshName) > 0)
		m_ObjectCount.at(p_MeshName)++;
	else
	{
		m_ObjectCount.insert(std::pair<std::string, int>(p_MeshName, 0));
	}

	ui->m_LightTree->addTopLevelItem(new TreeItem(p_MeshName + "_" + std::to_string(m_ObjectCount.at(p_MeshName)), p_ActorId));
}

void MainWindow::on_particleCreated_triggered(std::string p_MeshName, int p_ActorId)
{
	if(m_ObjectCount.count(p_MeshName) > 0)
		m_ObjectCount.at(p_MeshName)++;
	else
	{
		m_ObjectCount.insert(std::pair<std::string, int>(p_MeshName, 0));
	}

	ui->m_ParticleTree->addTopLevelItem(new TreeItem(p_MeshName + "_" + std::to_string(m_ObjectCount.at(p_MeshName)), p_ActorId));
}

void MainWindow::on_m_LightTreeAddButton_clicked()
{
	addFilter(ui->m_LightTree);
}

void MainWindow::on_m_ParticleTreeAddButton_clicked()
{
	addFilter(ui->m_ParticleTree);
}

void MainWindow::on_m_LightTreeRemoveButton_clicked()
{
    QTreeWidgetItem *currItem = ui->m_LightTree->currentItem();
    removeChild(currItem);
}

void MainWindow::on_m_ParticleTreeRemoveButton_4_clicked()
{
    QTreeWidgetItem *currItem = ui->m_ParticleTree->currentItem();
    removeChild(currItem);
}

void MainWindow::setObjectScale()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();
	if(currItem)
	{
		TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);

		if(currItem->isSelected() && cItem)
        {
			Actor::ptr actor = ui->m_RenderWidget->getObjectManager()->getActor(cItem->getActorId());
			std::weak_ptr<ModelComponent> pmodel = actor->getComponent<ModelComponent>(ModelInterface::m_ComponentId);
            std::shared_ptr<ModelComponent> spmodel = pmodel.lock();

            if(spmodel)
            {
                spmodel->setScale(Vector3(ui->m_ObjectScaleXBox->value(),ui->m_ObjectScaleYBox->value(),ui->m_ObjectScaleZBox->value()));
            }
		}
	}
}

void MainWindow::setObjectPosition()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();
    if(currItem)
    {
        TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);

        if(currItem->isSelected() && cItem)
        {
            Actor::ptr actor = ui->m_RenderWidget->getObjectManager()->getActor(cItem->getActorId());
            std::weak_ptr<ModelComponent> pmodel = actor->getComponent<ModelComponent>(ModelInterface::m_ComponentId);
            std::shared_ptr<ModelComponent> spmodel = pmodel.lock();

            if(spmodel)
            {
                spmodel->setPosition(Vector3(ui->m_ObjectPositionXBox->value(),ui->m_ObjectPositionYBox->value(),ui->m_ObjectPositionZBox->value()));

            }
        }
    }
}

void MainWindow::setObjectRotation()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();
    if(currItem)
    {
        TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);

        if(currItem->isSelected() && cItem)
        {
            Actor::ptr actor = ui->m_RenderWidget->getObjectManager()->getActor(cItem->getActorId());
            std::weak_ptr<ModelComponent> pmodel = actor->getComponent<ModelComponent>(ModelInterface::m_ComponentId);
            std::shared_ptr<ModelComponent> spmodel = pmodel.lock();

            if(spmodel)
            {
                spmodel->setRotation(Vector3(ui->m_ObjectRotationXBox->value(),ui->m_ObjectRotationYBox->value(),ui->m_ObjectRotationZBox->value()));
            }
        }
    }
}

void MainWindow::createSimpleObjectDescription(const std::string& p_ModelName)
{
	ActorFactory::InstanceModel model;
	model.meshName = p_ModelName;
	model.position = Vector3();
	model.rotation = Vector3();
	model.scale = Vector3(1.f, 1.f, 1.f);

	std::vector<ActorFactory::InstanceBoundingVolume> volumes;
	ActorFactory::InstanceBoundingVolume volume;
	volume.meshName = p_ModelName;
	volume.scale = Vector3(1.f, 1.f, 1.f);
	volumes.push_back(volume);

	std::vector<ActorFactory::InstanceEdgeBox> edges;

	ui->m_RenderWidget->registerObjectDescription(p_ModelName, ActorFactory::getInstanceActorDescription(model, volumes, edges));
}

void MainWindow::addSimpleObjectType(const std::string& p_ModelName)
{
    QTableWidgetItem *item = new QTableWidgetItem();
	item->setIcon(m_DefaultObjectIcon);
	item->setText(QString::fromStdString(p_ModelName));
    item->setTextAlignment(Qt::AlignBottom | Qt::AlignCenter);

	int column = ui->m_ObjectTable->columnCount();
	ui->m_ObjectTable->insertColumn(column);
	ui->m_ObjectTable->setItem(0, column, item);
	createSimpleObjectDescription(p_ModelName);
}
