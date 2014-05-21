#include "qfilesystemmodeldialog.h"

#include <QStandardPaths>
#include <string>

QFileSystemModelDialog::QFileSystemModelDialog(QWidget *parent) :
    QDialog(parent)
{
    dirModel = new QFileSystemModel(this);
	fileModel = new QFileSystemModel(this);
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
	
	// Directory model stuff
    // Set filter
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);

    // QFileSystemModel -requires- root path
	QString startPath("./assets/Objects");
    dirModel->setRootPath(startPath);
	QModelIndex ind = dirModel->index(startPath);
	m_DirView->setRootIndex(ind);

	//File model stuff
	// Set filter
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);

    // QFileSystemModel requires root path
    fileModel->setRootPath(startPath);
	ind = fileModel->index(startPath);
	m_FileView->setRootIndex(ind);
}

void QFileSystemModelDialog::attachViews()
{
	// Attach the model to the view
    m_DirView->setModel(dirModel);
	m_FileView->setModel(fileModel);
}

void QFileSystemModelDialog::directoryViewClicked(const QModelIndex &index)
{
    QString mPath = dirModel->fileInfo(index).absoluteFilePath();
	
    fileModel->setRootPath(mPath);
    m_FileView->setRootIndex(fileModel->index(mPath));
}

// http://www.bogotobogo.com/Qt/Qt5_QTreeView_QFileSystemModel_ModelView_MVC.php
