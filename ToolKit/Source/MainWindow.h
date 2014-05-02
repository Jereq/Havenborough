#pragma once

#include <QMainWindow>
#include <QTimer>
#include "Utilities\XMFloatUtil.h"
#include <QTreeWidgetItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	Ui::MainWindow *ui;
	QTimer m_Timer;
    void removeChild(QTreeWidgetItem* currItem);
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

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

signals:
    void setCameraPositionSignal(Vector3 p_CameraPosition);
};
