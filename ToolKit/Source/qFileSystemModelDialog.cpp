#include "qfilesystemmodeldialog.h"
#include "ui_qfilesystemmodeldialog.h"

QFileSystemModelDialog::QFileSystemModelDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QFileSystemModelDialog)
{
    ui->setupUi(this);

    // Creates our new model and populate
    QString mPath = "C:/";
    dirModel = new QFileSystemModel(this);

    // Set filter
    dirModel->setFilter(QDir::NoDotAndDotDot |
                        QDir::AllDirs);

    // QFileSystemModel requires root path
    dirModel->setRootPath(mPath);

    // Attach the model to the view
    ui->treeView->setModel(dirModel);
}

QFileSystemModelDialog::~QFileSystemModelDialog()
{
    delete ui;
}


// http://www.bogotobogo.com/Qt/Qt5_QTreeView_QFileSystemModel_ModelView_MVC.php
