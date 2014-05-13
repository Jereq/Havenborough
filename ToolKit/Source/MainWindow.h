#pragma once

#include <QMainWindow>
#include <QTimer>

#include <Actor.h>
#include <EventManager.h>
#include <Utilities\XMFloatUtil.h>

#ifndef Q_MOC_RUN
#include <ResourceManager.h>
#endif

class QTableWidgetItem;
class QTreeWidgetItem;

class AnimationLoader;
class IGraphics;
class IPhysics;
class ObjectManager;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	Ui::MainWindow *ui;
	QTimer m_Timer;
	QIcon m_DefaultObjectIcon;

	EventManager m_EventManager;
	ResourceManager m_ResourceManager;
	std::unique_ptr<ObjectManager> m_ObjectManager;
	std::unique_ptr<AnimationLoader> m_AnimationLoader;
	IGraphics* m_Graphics;
	IPhysics* m_Physics;
	
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	void signalAndSlotsDefinitions();

private slots:
    void on_actionObject_Tree_triggered();

	void idle();

    void on_actionExit_triggered();

    void on_actionOpen_triggered();

    void on_actionSave_triggered();
	
	void splitCameraPosition(Vector3 p_cameraPosition);

    void setCameraPosition();

    void on_actionProperties_triggered();

    void on_actionAdd_Object_triggered();

    void on_m_ObjectTree_itemSelectionChanged();

    void on_actionParticle_Tree_triggered();

    void on_actionLight_Tree_triggered();

    void setObjectScale();

    void setObjectRotation();

    void setObjectPosition();

	void addObject(QTableWidgetItem* p_ObjectItem);

    void on_m_ObjectTree_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void onActorAdded(std::string p_ObjectType, Actor::ptr p_Actor);

signals:
    void setCameraPositionSignal(Vector3 p_CameraPosition);

private:
	void onFrame(float p_DeltaTime);
	void loadLevel(const std::string& p_Filename);

	void initializeSystems();
	void uninitializeSystems();
	void pick(IEventData::Ptr p_Data);
};
