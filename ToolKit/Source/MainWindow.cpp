#include "MainWindow.h"
#include "ui_MainWindow.h"

#ifdef _DEBUG
#pragma comment(lib, "Commond.lib")
#pragma comment(lib, "Graphicsd.lib")
#pragma comment(lib, "Physicsd.lib")
#else
#pragma comment(lib, "Common.lib")
#pragma comment(lib, "Graphics.lib")
#pragma comment(lib, "Physics.lib")
#endif
#pragma comment(lib, "d3d11.lib")

#include <QFileDialog>

#include <ActorFactory.h>
#include <Components.h>
#include <TweakSettings.h>

#include "ObjectManager.h"
#include "TreeItem.h"
#include "TreeFilter.h"
#include "TableItem.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	m_DefaultObjectIcon(":/Icons/Assets/object.png"),
	m_Physics(nullptr)
{
	ui->setupUi(this);

	initializeSystems();

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

	QObject::connect(m_ObjectManager.get(), SIGNAL(meshCreated(std::string, int)), this, SLOT(on_meshCreated_triggered(std::string, int)));
	QObject::connect(m_ObjectManager.get(), SIGNAL(lightCreated(std::string, int)), this, SLOT(on_lightCreated_triggered(std::string, int)));
	QObject::connect(m_ObjectManager.get(), SIGNAL(particleCreated(std::string, int)), this, SLOT(on_particleCreated_triggered(std::string, int)));

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
	uninitializeSystems();
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
	float deltaTime = m_Timer.interval() / 1000.f;
	onFrame(deltaTime);
	ui->m_RenderWidget->onFrame(deltaTime);
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
		
		loadLevel(fullFilePath.toStdString());
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

			Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
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
			Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
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
			Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
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
			Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
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

	registerObjectDescription(p_ModelName, ActorFactory::getInstanceActorDescription(model, volumes, edges));
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

void MainWindow::onFrame(float p_DeltaTime)
{
	m_EventManager.processEvents();
	
	m_ObjectManager->update(p_DeltaTime);
}

void MainWindow::loadLevel(const std::string& p_Filename)
{
	m_ObjectManager->loadLevel(p_Filename);
}

void MainWindow::registerObjectDescription(const std::string& p_ObjectName, const std::string& p_Description)
{
	m_ObjectManager->registerObjectDescription(p_ObjectName, p_Description);
}

void MainWindow::addObject(QTableWidgetItem* p_ObjectItem)
{
	m_ObjectManager->addObject(p_ObjectItem->text().toStdString(), Vector3(rand() % 1000, rand() % 1000, rand() % 1000));
}

void MainWindow::initializeSystems()
{
	TweakSettings::initializeMaster();

	m_Physics = IPhysics::createPhysics();
	m_Physics->initialize(false, 1.f / 60.f);

	m_Graphics = IGraphics::createGraphics();
	m_Graphics->setTweaker(TweakSettings::getInstance());
	m_Graphics->setShadowMapResolution(1024);
	m_Graphics->enableShadowMap(true);
	m_Graphics->initialize((HWND)ui->m_RenderWidget->winId(), width(), height(), false, 60.f);

	m_Graphics->setLoadModelTextureCallBack(&ResourceManager::loadModelTexture, &m_ResourceManager);
	m_Graphics->setReleaseModelTextureCallBack(&ResourceManager::releaseModelTexture, &m_ResourceManager);

	using std::placeholders::_1;
	using std::placeholders::_2;
	m_ResourceManager.registerFunction("model",
		std::bind(&IGraphics::createModel, m_Graphics, _1, _2),
		std::bind(&IGraphics::releaseModel, m_Graphics, _1) );
	m_ResourceManager.registerFunction("texture",
		std::bind(&IGraphics::createTexture, m_Graphics, _1, _2),
		std::bind(&IGraphics::releaseTexture, m_Graphics, _1));
	m_ResourceManager.registerFunction("volume",
		std::bind(&IPhysics::createBV, m_Physics, _1, _2),
		std::bind(&IPhysics::releaseBV, m_Physics, _1));
	m_ResourceManager.registerFunction("particleSystem",
		std::bind(&IGraphics::createParticleEffectDefinition, m_Graphics, _1, _2),
		std::bind(&IGraphics::releaseParticleEffectDefinition, m_Graphics, _1));
	m_ResourceManager.loadDataFromFile("assets/Resources.xml");

	ActorFactory::ptr actorFactory(new ActorFactory(0));
	actorFactory->setPhysics(m_Physics);
	actorFactory->setEventManager(&m_EventManager);
	actorFactory->setResourceManager(&m_ResourceManager);
	m_ObjectManager.reset(new ObjectManager(actorFactory, &m_EventManager, &m_ResourceManager));

	ui->m_RenderWidget->initialize(&m_EventManager, &m_ResourceManager, m_Graphics);
}

void MainWindow::uninitializeSystems()
{
	m_ResourceManager.setReleaseImmediately(true);

	m_ObjectManager.reset();

	ui->m_RenderWidget->uninitialize();

	m_ResourceManager.unregisterResourceType("model");
	m_ResourceManager.unregisterResourceType("texture");

	if (m_Graphics)
	{
		IGraphics::deleteGraphics(m_Graphics);
		m_Graphics = nullptr;
	}

	if (m_Physics)
	{
		IPhysics::deletePhysics(m_Physics);
		m_Physics = nullptr;
	}
}
