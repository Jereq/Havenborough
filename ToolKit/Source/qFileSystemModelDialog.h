#ifndef QFILESYSTEMMODELDIALOG_H
#define QFILESYSTEMMODELDIALOG_H

#include <QDialog>
#include <QFileSystemModel>
#include <QTreeWidgetItem>
#include <QListView>

namespace Ui {
class QFileSystemModelDialog;
}

class QFileSystemModelDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QFileSystemModelDialog(QWidget* parent);
    ~QFileSystemModelDialog();

	void setViews(QTreeView* dirView, QListView* fileView);
	void attachDirectoryView(QTreeView* target);

    void directoryViewClicked(const QModelIndex &index);

private:
	void attachViews();

private:
    // Make two models instead of one to filter them separately
    QFileSystemModel *dirModel;
    QFileSystemModel *fileModel;

	// Get pointers to the views from the MainWindow
	QTreeView* m_DirView;
	QListView* m_FileView;
};

#endif // QFILESYSTEMMODELDIALOG_H
