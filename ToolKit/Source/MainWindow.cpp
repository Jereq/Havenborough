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

#include <EventData.h>


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

    //Hide all value boxes per default
	hideItemProperties();

	signalAndSlotsDefinitions();
    pushBoxes();
	
    ui->m_HelpWidget->hide();
    ui->m_HelpWidget->setFloating(true);

    //Timer
	m_Timer.setInterval(1000 / 60);
	m_Timer.setSingleShot(false);
	QObject::connect(&m_Timer, SIGNAL(timeout()), this, SLOT(idle()));
	m_Timer.start();

	m_ObjectManager->loadDescriptionsFromFolder("assets/Objects");

	initializeHotkeys();
}

MainWindow::~MainWindow()
{
	for(auto &key : m_Hotkey)
	{
		delete key.second;
		key.second = nullptr;
	}
	uninitializeSystems();
	delete ui;
}

void MainWindow::signalAndSlotsDefinitions()
{
	//Signals and slots for connecting the camera to the camerabox values
    QObject::connect(ui->m_RenderWidget, SIGNAL(CameraPositionChanged(Vector3)), this, SLOT(splitCameraPosition(Vector3)));

    QObject::connect(ui->m_CameraPositionXBox, SIGNAL(editingFinished()), this, SLOT(setCameraPosition()));
    QObject::connect(ui->m_CameraPositionYBox, SIGNAL(editingFinished()), this, SLOT(setCameraPosition()));
    QObject::connect(ui->m_CameraPositionZBox, SIGNAL(editingFinished()), this, SLOT(setCameraPosition()));

    QObject::connect(this, SIGNAL(setCameraPositionSignal(Vector3)), ui->m_RenderWidget, SLOT(CameraPositionSet(Vector3)));
	QObject::connect(&m_CamInt, SIGNAL(setCameraPositionSignal(Vector3)), ui->m_RenderWidget, SLOT(CameraPositionSet(Vector3)));

    //Signals and slots for connecting the object creation to the trees
	QObject::connect(m_ObjectManager.get(), SIGNAL(actorAdded(std::string,  Actor::ptr)), this, SLOT(onActorAdded(std::string, Actor::ptr)));

    //Signals and slots for connecting the object scale editing to the object
    QObject::connect(ui->m_ObjectScaleXBox, SIGNAL(editingFinished()), this, SLOT(setObjectScale()));
    QObject::connect(ui->m_ObjectScaleYBox, SIGNAL(editingFinished()), this, SLOT(setObjectScale()));
    QObject::connect(ui->m_ObjectScaleZBox, SIGNAL(editingFinished()), this, SLOT(setObjectScale()));

    //Signals and slots for connecting the object rotation editing to the object
    QObject::connect(ui->m_ObjectRotationXBox, SIGNAL(editingFinished()), this, SLOT(setObjectRotation()));
    QObject::connect(ui->m_ObjectRotationYBox, SIGNAL(editingFinished()), this, SLOT(setObjectRotation()));
    QObject::connect(ui->m_ObjectRotationZBox, SIGNAL(editingFinished()), this, SLOT(setObjectRotation()));

    //Signals and slots for connecting the object position editing to the object
    QObject::connect(ui->m_ObjectPositionXBox, SIGNAL(editingFinished()), this, SLOT(setObjectPosition()));
    QObject::connect(ui->m_ObjectPositionYBox, SIGNAL(editingFinished()), this, SLOT(setObjectPosition()));
    QObject::connect(ui->m_ObjectPositionZBox, SIGNAL(editingFinished()), this, SLOT(setObjectPosition()));

    //Signals and slots for connecting the Filter creation to the trees
	QObject::connect(ui->m_ObjectTreeAddButton, SIGNAL(clicked()), ui->m_ObjectTree, SLOT(addFilter()));

    //Signals and slots for connecting the remove button creation to the trees
	QObject::connect(ui->m_ObjectTreeRemoveButton, SIGNAL(clicked()), ui->m_ObjectTree, SLOT(removeItem()));

	//Signals and slots for connecting the Tree item creation to the table
	QObject::connect(m_ObjectManager.get(), SIGNAL(objectTypeCreated(std::string)), ui->m_ObjectTable, SLOT(addObject(std::string)));

    //Signals and slots for connecting the light position editing to the light
    QObject::connect(ui->m_LightPositionXBox, SIGNAL(editingFinished()), this, SLOT(setLightPosition()));
    QObject::connect(ui->m_LightPositionYBox, SIGNAL(editingFinished()), this, SLOT(setLightPosition()));
    QObject::connect(ui->m_LightPositionZBox, SIGNAL(editingFinished()), this, SLOT(setLightPosition()));

	//Signals and slots for connecting the light color editing to the light
    QObject::connect(ui->m_LightColorXBox, SIGNAL(editingFinished()), this, SLOT(setLightColor()));
    QObject::connect(ui->m_LightColorYBox, SIGNAL(editingFinished()), this, SLOT(setLightColor()));
    QObject::connect(ui->m_LightColorZBox, SIGNAL(editingFinished()), this, SLOT(setLightColor()));

	//Signals and slots for connecting the light direction editing to the light
    QObject::connect(ui->m_LightDirectionXBox, SIGNAL(editingFinished()), this, SLOT(setLightDirection()));
    QObject::connect(ui->m_LightDirectionYBox, SIGNAL(editingFinished()), this, SLOT(setLightDirection()));
    QObject::connect(ui->m_LightDirectionZBox, SIGNAL(editingFinished()), this, SLOT(setLightDirection()));

	//Signals and slots for connecting the light angles editing to the light
    QObject::connect(ui->m_LightAngleXBox, SIGNAL(editingFinished()), this, SLOT(setLightAngles()));
    QObject::connect(ui->m_LightAngleYBox, SIGNAL(editingFinished()), this, SLOT(setLightAngles()));

	//Signals and slots for connecting the light range editing to the light
    QObject::connect(ui->m_LightAdditionalBox2, SIGNAL(editingFinished()), this, SLOT(setLightRange()));

	//Signals and slots for connecting the light intensity editing to the light
    QObject::connect(ui->m_LightAdditionalBox1, SIGNAL(editingFinished()), this, SLOT(setLightIntensity()));

	//Signal and slot for removing actor when pressing remove in the tree.
	QObject::connect(ui->m_ObjectTree, SIGNAL(removeActor(int)), m_ObjectManager.get(), SLOT(actorRemoved(int)));

	QObject::connect(ui->m_ObjectTree, SIGNAL(deselectAll()), this, SLOT(deselectAllTreeItems()));
	QObject::connect(this, SIGNAL(deselectAll()), this, SLOT(deselectAllTreeItems()));
}

void MainWindow::pushBoxes()
{
    m_Boxes.push_back(ui->PositionBox);
    m_Boxes.push_back(ui->PositionBox_2);
    m_Boxes.push_back(ui->ScaleBox);
    m_Boxes.push_back(ui->RotationBox);

    m_Boxes.push_back(ui->ColorBox);
    m_Boxes.push_back(ui->DirectionBox);
    m_Boxes.push_back(ui->AngleBox);
    m_Boxes.push_back(ui->AdditionalBox_1);
    m_Boxes.push_back(ui->AdditionalBox_2);
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
	QString fullFilePath = QFileDialog::getOpenFileName(this, tr("Open Level"), "./assets/levels/", tr("Level Files (*.xml *.btxl)"));
	if (!fullFilePath.isNull())
	{
		loadLevel(fullFilePath.toStdString());
	}
}

void MainWindow::on_actionSave_triggered()
{
    QString fullFilePath = QFileDialog::getSaveFileName(this, tr("Save Level As..."), "./assets/levels/", tr("Level Files (*.xml"));
	if (!fullFilePath.isNull())
	{
		saveLevel(fullFilePath.toStdString());
	}
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
	QList<QTreeWidgetItem*> selectedItems = ui->m_ObjectTree->selectedItems();
	if(selectedItems.empty())
		return;
	emit deselectAll();

	for(auto &item : selectedItems)
	{
		TreeFilter *treeFilter = dynamic_cast<TreeFilter*>(item);
		if(!treeFilter)
			continue;
		
		ui->m_ObjectTree->selectAllChilds(item, QItemSelectionModel::Select);
	}

	selectedItems = ui->m_ObjectTree->selectedItems();

	for(auto &item : selectedItems)
	{
		TreeItem *treeItem = dynamic_cast<TreeItem*>(item);
		if(!treeItem)
			continue;

		Actor::ptr actor = m_ObjectManager->getActor(treeItem->getActorId());
		if(!actor)
			continue;
		std::weak_ptr<ModelComponent> wpModel = actor->getComponent<ModelComponent>(ModelInterface::m_ComponentId);
		std::shared_ptr<ModelComponent> spModel = wpModel.lock();

		if(!spModel)
			continue;
		spModel->setColorTone(Vector3(5.0f, 5.0f, 7.0f));
	}

	itemPropertiesChanged();
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
	using namespace DirectX;

	QList<QTreeWidgetItem*> selectedItems = ui->m_ObjectTree->selectedItems();
	Vector3 newPos = Vector3(ui->m_ObjectPositionXBox->value(),ui->m_ObjectPositionYBox->value(),ui->m_ObjectPositionZBox->value());

	if(newPos == previousPosition)
		return;

	previousPosition = newPos;

	Vector3 originalPosition = findMiddlePoint(selectedItems);
	XMVECTOR difference = XMLoadFloat3(&newPos) - XMLoadFloat3(&originalPosition);

	for( auto *item : selectedItems)
	{
		TreeItem *cItem = dynamic_cast<TreeItem*>(item);
		if(!cItem)
			continue;

		Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
		if(!actor)
			continue;

		XMVECTOR oldPosition = XMLoadFloat3(&actor->getPosition());
		oldPosition += difference;
		Vector3 newPosition;
		XMStoreFloat3(&newPosition, oldPosition);
		actor->setPosition(newPosition);
	}
	


   // QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();

   // if(currItem)
   // {
   //     TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);

   //     if(currItem->isSelected() && cItem && cItem->getType() == TreeItem::TreeItemType::MODEL)
   //     {
			//Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
			//actor->setPosition(Vector3(ui->m_ObjectPositionXBox->value(),ui->m_ObjectPositionYBox->value(),ui->m_ObjectPositionZBox->value()));
   //     }
   // }
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
			actor->setRotation(Vector3(ui->m_ObjectRotationXBox->value(),ui->m_ObjectRotationYBox->value(),ui->m_ObjectRotationZBox->value()));
        }
    }
}

void MainWindow::setLightPosition()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();

	if(currItem)
	{
		TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);
        if(currItem->isSelected() && cItem)
		{
			Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
			actor->setPosition(Vector3(ui->m_LightPositionXBox->value(),ui->m_LightPositionYBox->value(),ui->m_LightPositionZBox->value()));
		}
	}
}

void MainWindow::setLightColor()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();

	if(currItem)
	{
		TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);
        if(currItem->isSelected() && cItem)
		{
			Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
			std::weak_ptr<LightComponent> light = actor->getComponent<LightComponent>(LightInterface::m_ComponentId);
			std::shared_ptr<LightComponent> slight = light.lock();
			slight->setColor(Vector3(ui->m_LightColorXBox->value(),ui->m_LightColorYBox->value(),ui->m_LightColorZBox->value()));
		}
	}
}

void MainWindow::setLightDirection()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();

	if(currItem)
	{
		TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);
        if(currItem->isSelected() && cItem)
		{
			Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
			std::weak_ptr<LightComponent> light = actor->getComponent<LightComponent>(LightInterface::m_ComponentId);
			std::shared_ptr<LightComponent> slight = light.lock();
			slight->setDirection(Vector3(ui->m_LightDirectionXBox->value(),ui->m_LightDirectionYBox->value(),ui->m_LightDirectionZBox->value()));
		}
	}
}

void MainWindow::setLightAngles()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();

	if(currItem)
	{
		TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);
        if(currItem->isSelected() && cItem)
		{
			Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
			std::weak_ptr<LightComponent> light = actor->getComponent<LightComponent>(LightInterface::m_ComponentId);
			std::shared_ptr<LightComponent> slight = light.lock();
			slight->setSpotLightAngles(Vector2(ui->m_LightAngleXBox->value(),ui->m_LightAngleYBox->value()));
		}
	}
}

void MainWindow::setLightRange()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();

	if(currItem)
	{
		TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);
        if(currItem->isSelected() && cItem)
		{
			Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
			std::weak_ptr<LightComponent> light = actor->getComponent<LightComponent>(LightInterface::m_ComponentId);
			std::shared_ptr<LightComponent> slight = light.lock();
			slight->setRange(ui->m_LightAdditionalBox2->value());
		}
	}
}

void MainWindow::setLightIntensity()
{
    QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();

	if(currItem)
	{
		TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);
        if(currItem->isSelected() && cItem)
		{
			Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
			std::weak_ptr<LightComponent> light = actor->getComponent<LightComponent>(LightInterface::m_ComponentId);
			std::shared_ptr<LightComponent> slight = light.lock();
			slight->setIntensity(ui->m_LightAdditionalBox1->value());
		}
	}
}

void MainWindow::deselectAllTreeItems()
{
	hideItemProperties();
	QList<TreeItem*> allItems = ui->m_ObjectTree->getAllTreeItems();
	for(auto &item : allItems)
	{
		Actor::ptr actor = m_ObjectManager->getActor(item->getActorId());
		if(!actor)
			continue;
		std::weak_ptr<ModelComponent> wpModel = actor->getComponent<ModelComponent>(ModelInterface::m_ComponentId);
		std::shared_ptr<ModelComponent> spModel = wpModel.lock();

		if(!spModel)
			continue;
		spModel->setColorTone(Vector3(1.0f, 1.0f, 1.0f));
	}
}

void MainWindow::shortcutDeselect(void)
{
	emit deselectAll();
	ui->m_ObjectTree->clearSelection();
}

void MainWindow::onFrame(float p_DeltaTime)
{
	m_EventManager.processEvents();
	
	m_ObjectManager->update(p_DeltaTime);

	m_CamInt.update(p_DeltaTime);
}

void MainWindow::loadLevel(const std::string& p_Filename)
{
	m_ObjectManager->loadLevel(p_Filename);
}

void MainWindow::saveLevel(const std::string& p_Filename)
{
	m_ObjectManager->saveLevel(p_Filename);
}

void MainWindow::addObject(QTableWidgetItem* p_ObjectItem)
{
	const Vector3& cameraPos = ui->m_RenderWidget->getCamera().getPosition();
	const Vector3& lookDir = ui->m_RenderWidget->getCamera().getForward();
	const Vector3 addPos = cameraPos + lookDir * 500.f;
	m_ObjectManager->addObject(p_ObjectItem->text().toStdString(), addPos);
}

void MainWindow::initializeSystems()
{
	TweakSettings::initializeMaster();

	m_Physics = IPhysics::createPhysics();
	m_Physics->initialize(false, 1.f / 60.f);

	m_AnimationLoader.reset(new AnimationLoader);

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
	m_ResourceManager.registerFunction("animation",
		std::bind(&AnimationLoader::loadAnimationDataResource, m_AnimationLoader.get(), _1, _2),
		std::bind(&AnimationLoader::releaseAnimationData, m_AnimationLoader.get(), _1));
	m_ResourceManager.loadDataFromFile("assets/Resources.xml");

	ActorFactory::ptr actorFactory(new ActorFactory(0));
	actorFactory->setPhysics(m_Physics);
	actorFactory->setEventManager(&m_EventManager);
	actorFactory->setResourceManager(&m_ResourceManager);
	actorFactory->setAnimationLoader(m_AnimationLoader.get());
	m_ObjectManager.reset(new ObjectManager(actorFactory, &m_EventManager, &m_ResourceManager, ui->m_ObjectTree));

	ui->m_RenderWidget->initialize(&m_EventManager, &m_ResourceManager, m_Graphics);

	m_EventManager.addListener(EventListenerDelegate(this, &MainWindow::pick), CreatePickingEventData::sk_EventType);
}

void MainWindow::uninitializeSystems()
{
	m_ResourceManager.setReleaseImmediately(true);

	m_ObjectManager.reset();

	ui->m_RenderWidget->uninitialize();

	m_ResourceManager.unregisterResourceType("animation");
	m_AnimationLoader.reset();

	m_ResourceManager.unregisterResourceType("model");
	m_ResourceManager.unregisterResourceType("particleSystem");
	m_ResourceManager.unregisterResourceType("texture");

	if (m_Graphics)
	{
		IGraphics::deleteGraphics(m_Graphics);
		m_Graphics = nullptr;
	}

	m_ResourceManager.unregisterResourceType("volume");

	if (m_Physics)
	{
		IPhysics::deletePhysics(m_Physics);
		m_Physics = nullptr;
	}
}

void MainWindow::initializeHotkeys()
{
	using std::map;
	using std::pair;
	using std::string;
	
	m_Hotkey.insert(pair<string, QShortcut*>("deselect_ctrl+d", new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_D), this)));
	QObject::connect(m_Hotkey["deselect_ctrl+d"], SIGNAL(activated()), this, SLOT(shortcutDeselect()));
	m_Hotkey.insert(pair<string, QShortcut*>("deselect_escape", new QShortcut(QKeySequence(Qt::Key_Escape), this)));
	QObject::connect(m_Hotkey["deselect_escape"], SIGNAL(activated()), this, SLOT(shortcutDeselect()));
}

void MainWindow::sortPropertiesBoxes()
{
    int x = 9;
    int incrementalY = 122;
    int marginY = 5;

    for(unsigned int i = 0; i < m_Boxes.size(); i++)
    {
        if(!m_Boxes.at(i)->isHidden())
        {
            m_Boxes.at(i)->move(x, incrementalY);
			incrementalY += m_Boxes.at(i)->height() + marginY;
        }
    }
}

void MainWindow::onActorAdded(std::string p_ObjectType, Actor::ptr p_Actor)
{
	std::string objectName = "Object";
	int type = TreeItem::TreeItemType::UNKNOWN;

	std::weak_ptr<LightComponent> lmodel = p_Actor->getComponent<LightComponent>(LightInterface::m_ComponentId);
	std::shared_ptr<LightComponent> slmodel = lmodel.lock();
	if(slmodel)
	{
		switch(slmodel->getType())
		{
		case LightClass::Type::DIRECTIONAL: objectName = "Directional"; break;
		case LightClass::Type::SPOT: objectName = "Spot"; break;
		case LightClass::Type::POINT: objectName = "Point"; break;
		}
		type = (int)slmodel->getType();
	}
	std::weak_ptr<ParticleComponent> pmodel = p_Actor->getComponent<ParticleComponent>(ParticleInterface::m_ComponentId);
	std::shared_ptr<ParticleComponent> spmodel = pmodel.lock();
	if(spmodel)
	{
		objectName = spmodel->getEffectName();
		type = 4;
	}
	std::weak_ptr<ModelComponent> model = p_Actor->getComponent<ModelComponent>(ModelInterface::m_ComponentId);
	std::shared_ptr<ModelComponent> smodel = model.lock();
	if(smodel)
	{
		objectName = smodel->getMeshName();
		type = 3;
	}

	if(!p_ObjectType.empty())
		objectName = p_ObjectType;

	ui->m_ObjectTree->objectCreated(objectName, p_Actor->getId(), type);
}

void MainWindow::pick(IEventData::Ptr p_Data)
{
	std::shared_ptr<CreatePickingEventData> data = std::static_pointer_cast<CreatePickingEventData>(p_Data);
	BodyHandle b = m_Physics->rayCast(data.get()->getRayDir(), data.get()->getRayOrigin());
	Actor::ptr actor = m_ObjectManager->getActorFromBodyHandle(b);
	if(!actor)
		return;

	ui->m_ObjectTree->selectItem(actor->getId());
}

void MainWindow::on_actionGo_To_Selected_triggered()
{
	QTreeWidgetItem *currItem = ui->m_ObjectTree->currentItem();
	if(!currItem)
	{
		m_CamInt.createPath(ui->m_RenderWidget->getCamera().getPosition(), Vector3(0,0,0), 0.5f);
		return;
	}

	TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);
	if(!cItem)
	{
		m_CamInt.createPath(ui->m_RenderWidget->getCamera().getPosition(), Vector3(0,0,0), 0.5f);
		return;
	}
	
    if(currItem->isSelected())
    {
		Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
		if(actor)
		{
			std::weak_ptr<BoundingMeshComponent> wBM = actor->getComponent<BoundingMeshComponent>(PhysicsInterface::m_ComponentId);
			if(!wBM.expired())
			{
				std::shared_ptr<BoundingMeshComponent> sBM = wBM.lock();
				float radius = m_Physics->getSurroundingSphereRadius(sBM->getBodyHandle());
				float fov = m_Graphics->getFOV();
				float distanceToCenter = radius / sinf(fov * 0.5f);
				
				using namespace DirectX;

				XMFLOAT4X4 view = m_Graphics->getView();

				XMVECTOR dir = XMVectorSet(view._31,view._32,view._33,0.f);
				XMVECTOR camPos, objectPos;
				objectPos = XMLoadFloat3(&actor->getPosition());

				camPos = objectPos + (-dir * distanceToCenter);
				XMFLOAT3 xmCamPos;
				XMStoreFloat3(&xmCamPos, camPos);

				m_CamInt.createPath(ui->m_RenderWidget->getCamera().getPosition(), xmCamPos, 0.5f);

				//emit setCameraPositionSignal(xmCamPos);
			}
			else
			{
				m_CamInt.createPath(ui->m_RenderWidget->getCamera().getPosition(), actor->getPosition(), 0.5f);
			}
		}
	}
	else
	{
		m_CamInt.createPath(ui->m_RenderWidget->getCamera().getPosition(), Vector3(0,0,0), 0.5f);
	}
}

void MainWindow::on_actionHelp_window_triggered()
{
    ui->m_HelpWidget->show();

    ui->m_HelpWidget->setGeometry(200, 200, 300, 500);
}

void MainWindow::itemPropertiesChanged(void)
{
	using namespace DirectX;

	QList<QTreeWidgetItem*> selectedItems = ui->m_ObjectTree->selectedItems();
	if(selectedItems.size() > 1)
	{
		hideItemProperties();

		ui->PositionBox->show();
		ui->ScaleBox->show();
		ui->RotationBox->show();


		Vector3 position = findMiddlePoint(selectedItems);

		ui->m_ObjectPositionXBox->setValue(position.x);
		ui->m_ObjectPositionYBox->setValue(position.y);
		ui->m_ObjectPositionZBox->setValue(position.z);
	}
	else
	{
		QTreeWidgetItem *currItem = selectedItems.front();
		if(!currItem)
			return;
	
		TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);
		if(!cItem)
			return;

		hideItemProperties();

		Actor::ptr actor = m_ObjectManager->getActor(cItem->getActorId());
		std::weak_ptr<ModelComponent> pmodel = actor->getComponent<ModelComponent>(ModelInterface::m_ComponentId);
		std::shared_ptr<ModelComponent> spmodel = pmodel.lock();

		if(currItem && currItem->isSelected())
		{
			if(currItem->isSelected() && spmodel)
			{
				ui->PositionBox->show();
				ui->ScaleBox->show();
				ui->RotationBox->show();
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
				spmodel->setColorTone(Vector3(5,5,7));
			}
			else if (spmodel)
			{
				spmodel->setColorTone(Vector3(1.0f, 1.0f, 1.0f));
			}

			std::weak_ptr<LightComponent> light = actor->getComponent<LightComponent>(LightInterface::m_ComponentId);
			std::shared_ptr<LightComponent> slight = light.lock();

			if(slight)
			{
				TreeItem *cItem = dynamic_cast<TreeItem*>(currItem);
				if(cItem)
				{
					ui->PositionBox_2->show();
					ui->ColorBox->show();

					const Vector3& pos = actor->getPosition();
					ui->m_LightPositionXBox->setValue(pos.x);
					ui->m_LightPositionYBox->setValue(pos.y);
					ui->m_LightPositionZBox->setValue(pos.z);

					const Vector3& color = slight->getColor();
					ui->m_LightColorXBox->setValue(color.x);
					ui->m_LightColorYBox->setValue(color.y);
					ui->m_LightColorZBox->setValue(color.z);

					TreeItem::TreeItemType type = cItem->getType();

					if(type == TreeItem::TreeItemType::POINTLIGHT)
					{
						ui->AdditionalBox_2->show();

						const float &range = slight->getRange();
						ui->m_LightAdditionalBox2->setValue(range);
					}
					else if(type == TreeItem::TreeItemType::DIRECTIONALLIGHT)
					{
						ui->AdditionalBox_1->show();
						ui->DirectionBox->show();

						const Vector3& dir = slight->getDirection();
						ui->m_LightDirectionXBox->setValue(dir.x);
						ui->m_LightDirectionYBox->setValue(dir.y);
						ui->m_LightDirectionZBox->setValue(dir.z);

						const float &intensity = slight->getIntensity();
						ui->m_LightAdditionalBox1->setValue(intensity);
					}
					else
					{
						ui->AdditionalBox_2->show();
						ui->DirectionBox->show();
						ui->AngleBox->show();

						const Vector3& dir = slight->getDirection();
						ui->m_LightDirectionXBox->setValue(dir.x);
						ui->m_LightDirectionYBox->setValue(dir.y);
						ui->m_LightDirectionZBox->setValue(dir.z);

						const Vector2 &angle = slight->getSpotLightAngles();
						ui->m_LightAngleXBox->setValue(angle.x);
						ui->m_LightAngleYBox->setValue(angle.y);

						const float &range = slight->getRange();
						ui->m_LightAdditionalBox2->setValue(range);
					}
				}
			}
		}
	}

    sortPropertiesBoxes();
}

void MainWindow::hideItemProperties(void)
{
	ui->PositionBox->hide();
	ui->ScaleBox->hide();
	ui->RotationBox->hide();

	ui->ColorBox->hide();
	ui->DirectionBox->hide();
	ui->AdditionalBox_1->hide();
	ui->AdditionalBox_2->hide();
	ui->AngleBox->hide();
	ui->PositionBox_2->hide();
}

Vector3 MainWindow::findMiddlePoint(QList<QTreeWidgetItem*> p_Items)
{
	using namespace DirectX;

	XMVECTOR sharedMidPoint;
	XMVECTOR minPoint = g_XMZero;
	XMVECTOR maxPoint = g_XMZero;
	XMVECTOR position;

	TreeItem *item = dynamic_cast<TreeItem*>(p_Items.front());
	if(!item)
		throw std::exception("No actor found when trying to find middle point");

	Actor::ptr actor = m_ObjectManager->getActor(item->getActorId());
	if(!actor)
		throw std::exception("No actor found when trying to find middle point");

	std::weak_ptr<BoundingMeshComponent> wBM = actor->getComponent<BoundingMeshComponent>(PhysicsInterface::m_ComponentId);
	if(wBM.expired())
		throw std::exception("The actor did not have a boundingMeshComponent when trying to find middle point");

	std::shared_ptr<BoundingMeshComponent> sBM = wBM.lock();	
	minPoint = maxPoint = XMLoadFloat3(&(m_Physics->getBodyPosition(sBM->getBodyHandle())));

	if(p_Items.size() > 1)
	{
		for(auto &widgetItem : p_Items)
		{
			TreeItem *item = dynamic_cast<TreeItem*>(widgetItem);
			if(!item)
				continue;

			Actor::ptr actor = m_ObjectManager->getActor(item->getActorId());
			if(!actor)
				continue;

			std::weak_ptr<BoundingMeshComponent> wBM = actor->getComponent<BoundingMeshComponent>(PhysicsInterface::m_ComponentId);
			if(wBM.expired())
				continue;

			std::shared_ptr<BoundingMeshComponent> sBM = wBM.lock();	
			position = XMLoadFloat3(&(m_Physics->getBodyPosition(sBM->getBodyHandle())));
			maxPoint = XMVectorMax(position, maxPoint);
			minPoint = XMVectorMin(position, minPoint);
		}
		sharedMidPoint = minPoint + (maxPoint - minPoint) * 0.5f;
	}
	else
		sharedMidPoint = maxPoint;
		
	Vector3 midPosition;
	XMStoreFloat3(&midPosition, sharedMidPoint);

	return midPosition;
}