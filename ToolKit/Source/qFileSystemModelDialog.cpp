#include "qfilesystemmodeldialog.h"

#include <QStandardPaths>
#include <string>

QFileSystemModelDialog::QFileSystemModelDialog(QWidget *parent) :
    QDialog(parent)
{
    dirModel = new QFileSystemModel(this);
}

QFileSystemModelDialog::~QFileSystemModelDialog()
{
}

void QFileSystemModelDialog::setViews(QTreeView* dirView, QListView* fileView)
{
	m_DirView = dirView;
	m_FileView = fileView;

	attachViews();
	m_DirView->hideColumn( 1 );
	m_DirView->hideColumn( 2 );
	m_DirView->hideColumn( 3 );

    // Set filter
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

    // QFileSystemModel requires root path
	QString startPath("./assets/Objects");
    dirModel->setRootPath(startPath);
	QModelIndex ind = dirModel->index(startPath);
	m_DirView->setRootIndex(ind);
}

void QFileSystemModelDialog::attachViews()
{
	// Attach the model to the view
    m_DirView->setModel(dirModel);
}

// http://www.bogotobogo.com/Qt/Qt5_QTreeView_QFileSystemModel_ModelView_MVC.php
