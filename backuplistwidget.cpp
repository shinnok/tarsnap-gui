#include "backuplistwidget.h"
#include "backuplistitem.h"

#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>

/*FIXME: Remove after QTBUG-40449 is fixed in Qt5*/
#ifdef Q_OS_OSX
// for file drops from Finder, working around QTBUG-40449
namespace Utils {
namespace Platform {
extern QUrl osxRefToUrl(const QUrl &url);
} // Platform
} // Utils
#endif

BackupListWidget::BackupListWidget(QWidget *parent):
    QListWidget(parent), _actionRemoveItems(this)
{
    _actionRemoveItems.setText(tr("Remove"));
    _actionRemoveItems.setToolTip(tr("Remove selected items"));
    addAction(&_actionRemoveItems);
    connect(&_actionRemoveItems, SIGNAL(triggered()), this, SLOT(removeSelectedItems()));
}

BackupListWidget::~BackupListWidget()
{
}

void BackupListWidget::addItemWithUrl(QUrl url)
{
    if(!url.isEmpty())
    {
        QString fileUrl = url.toLocalFile();
        if(fileUrl.isEmpty())
            return;
        QFileInfo file(fileUrl);
        if(!file.exists())
            return;
        BackupListItem *item = new BackupListItem(url);
        connect(item, SIGNAL(requestDelete()), this, SLOT(removeItem()));
        this->insertItem(this->count(), item);
        this->setItemWidget(item, item->widget());
    }
}

void BackupListWidget::addItemsWithUrls(QList<QUrl> urls)
{
    foreach (QUrl url, urls) {
#ifdef Q_OS_OSX
/*FIXME: Remove after QTBUG-40449 is fixed in Qt5*/
        url = Utils::Platform::osxRefToUrl(url);
#endif
        addItemWithUrl(url);
    }
}

void BackupListWidget::removeItem()
{
//    QWidget *widget = this->itemWidget(qobject_cast<BackupListItem*>(sender()));
    QListWidgetItem *item = this->takeItem(this->row(qobject_cast<BackupListItem*>(sender())));
    Q_UNUSED(item)
    BackupListItem *backupItem = qobject_cast<BackupListItem*>(sender());
    backupItem->cleanup();
    delete backupItem;
}

void BackupListWidget::removeSelectedItems()
{
    foreach (QListWidgetItem *item, this->selectedItems())
    {
        if(item->isSelected())
        {
            QListWidgetItem *takenItem = this->takeItem(this->row(item));
            BackupListItem *backupItem = dynamic_cast<BackupListItem*>(takenItem);
            backupItem->cleanup();
            delete backupItem;
        }
    }
}

void BackupListWidget::dragMoveEvent( QDragMoveEvent* event )
{
    if ( !event->mimeData()->hasUrls() )
    {
        event->ignore();
        return;
    }
    event->accept();
}

void BackupListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void BackupListWidget::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (!urls.isEmpty())
        addItemsWithUrls(urls);

    event->acceptProposedAction();
}

void BackupListWidget::keyReleaseEvent(QKeyEvent *event)
{
    if((event->key() == Qt::Key_Delete) || (event->key() == Qt::Key_Backspace))
    {
        removeSelectedItems();
    }
    else
    {
        QListWidget::keyReleaseEvent(event);
    }
}

