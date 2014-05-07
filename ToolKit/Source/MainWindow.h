#pragma once

#include <map>

#include <QMainWindow>
#include <QTimer>

#include <EventManager.h>
#include <Utilities\XMFloatUtil.h>

#ifndef Q_MOC_RUN
#include <ResourceManager.h>
#endif

class QTableWidgetItem;
class QTreeWidget;
class QTreeWidgetItem;

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
	IGraphics* m_Graphics;
	IPhysics* m_Physics;

	std::map<std::string, int> m_ObjectCount;

	struct intVec2
	{
		unsigned int x;
		unsigned int y;

		intVec2()
		{
			x = y = 0;
		}
	};
	intVec2 m_TableIndex;

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
    void removeChild(QTreeWidgetItem* currItem);
	void addFilter(QTreeWidget* tree);

private slots:
    void on_m_ObjectTreeAddButton_clicked();

    void on_m_ObjectTreeRemoveButton_clicked();

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

	void on_meshCreated_triggered(std::string p_MeshName, int p_ActorId);

	void on_lightCreated_triggered(std::string p_LightName, int p_ActorId);

	void on_particleCreated_triggered(std::string p_ParticleName, int p_ActorId);

    void on_actionParticle_Tree_triggered();

    void on_actionLight_Tree_triggered();

    void on_m_LightTreeAddButton_clicked();

    void on_m_ParticleTreeAddButton_clicked();

    void on_m_LightTreeRemoveButton_clicked();

    void on_m_ParticleTreeRemoveButton_4_clicked();

    void setObjectScale();

    void setObjectRotation();

    void setObjectPosition();

	void addObject(QTableWidgetItem* p_ObjectItem);

signals:
    void setCameraPositionSignal(Vector3 p_CameraPosition);

private:
	void createSimpleObjectDescription(const std::string& p_ModelName);
	void addSimpleObjectType(const std::string& p_ModelName);
	void onFrame(float p_DeltaTime);
	void loadLevel(const std::string& p_Filename);
	void registerObjectDescription(const std::string& p_ObjectName, const std::string& p_Description);

	void initializeSystems();
	void uninitializeSystems();
};
