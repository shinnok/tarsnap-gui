#include "customfilesystemmodel.h"

CustomFileSystemModel::CustomFileSystemModel()
{
}

CustomFileSystemModel::~CustomFileSystemModel()
{
}

QList<QPersistentModelIndex> CustomFileSystemModel::checkedIndexes()
{
    return _checklist.toList();
}

Qt::ItemFlags CustomFileSystemModel::flags(const QModelIndex &index) const
{
    return QFileSystemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QVariant CustomFileSystemModel::data(const QModelIndex &index, int role) const
{
    if(role == Qt::CheckStateRole && index.column() == 0)
    {
        if(_checklist.contains(index))
        {
            return Qt::Checked;
        }
        else if(_partialChecklist.contains(index))
        {
            return Qt::PartiallyChecked;
        }
        else
        {
            // Return PartiallyChecked if any ancestor is checked.
            QModelIndex parent = index.parent();
            while(parent.isValid())
            {
                if(_checklist.contains(parent))
                    return Qt::PartiallyChecked;
                parent = parent.parent();
            }
        }
        return Qt::Unchecked;
    }
    return QFileSystemModel::data(index, role);
}

QVariant CustomFileSystemModel::dataInternal(const QModelIndex &index) const
{
    if(_checklist.contains(index))
        return Qt::Checked;
    else if(_partialChecklist.contains(index))
        return Qt::PartiallyChecked;
    else
        return Qt::Unchecked;
}

void CustomFileSystemModel::setIndexCheckState(const QModelIndex &index,
                                               const Qt::CheckState state)
{
    if(index.data(Qt::CheckStateRole) != state)
        setData(index, state, Qt::CheckStateRole);
}

bool CustomFileSystemModel::hasCheckedSibling(const QModelIndex &index)
{
    for(int i = 0; i < rowCount(index.parent()); i++)
    {
        QModelIndex sibling = index.sibling(i, index.column());
        if(sibling.isValid())
        {
            if(sibling == index)
                continue;
            if(data(sibling, Qt::CheckStateRole) == Qt::Checked)
                return true;
        }
    }
    return false;
}

bool CustomFileSystemModel::setData(const QModelIndex &index,
                                    const QVariant &value, int role)
{
    QVector<int> selectionChangedRole;
    selectionChangedRole << SELECTION_CHANGED_ROLE;
    if(role == Qt::CheckStateRole)
    {
        if(value == Qt::Checked)
        {
            _partialChecklist.remove(index);
            _checklist.insert(index);
            emit dataChanged(index, index, selectionChangedRole);

            QModelIndex parent = index.parent();
            if(parent.isValid())
            {
                if(parent.data(Qt::CheckStateRole) == Qt::Checked)
                {
                    // Set any partially-selected siblings to be unchecked.
                    for(int i = 0; i < rowCount(parent); i++)
                    {
                        QModelIndex child = parent.child(i, index.column());
                        if(child.isValid())
                        {
                            if(child == index)
                                continue;
                            if(data(child, Qt::CheckStateRole) ==
                                    Qt::PartiallyChecked)
                                setIndexCheckState(child, Qt::Unchecked);
                        }
                    }
                }
                // Set parent to be PartiallyChecked.
                setIndexCheckState(parent, Qt::PartiallyChecked);
            }
            if(isDir(index))
            {
                // Set all children to be PartiallyChecked.
                for(int i = 0; i < rowCount(index); i++)
                {
                    QModelIndex child = index.child(i, index.column());
                    if(child.isValid())
                        setData(child, Qt::PartiallyChecked,
                                Qt::CheckStateRole);
                }
            }
        }
        else if(value == Qt::PartiallyChecked)
        {
            if(_checklist.remove(index))
                emit dataChanged(index, index, selectionChangedRole);
            _partialChecklist.insert(index);
            QModelIndex parent = index.parent();
            if(parent.isValid() &&
               (parent.data(Qt::CheckStateRole) == Qt::Unchecked))
                setIndexCheckState(parent, Qt::PartiallyChecked);
        }
        else if(value == Qt::Unchecked)
        {
            _partialChecklist.remove(index);
            if(_checklist.remove(index))
            {
                emit dataChanged(index, index, selectionChangedRole);
                if(isDir(index))
                {
                    // Set all children to be unchecked.
                    for(int i = 0; i < rowCount(index); i++)
                    {
                        QModelIndex child = index.child(i, index.column());
                        if(child.isValid())
                            setData(child, Qt::Unchecked, Qt::CheckStateRole);
                    }
                }
            }
            QModelIndex parent = index.parent();
            if(parent.isValid())
            {
                if(hasCheckedSibling(index))
                    setIndexCheckState(parent, Qt::PartiallyChecked);
                else
                    setIndexCheckState(parent, Qt::Unchecked);
            }
        }
        QVector<int> roles;
        roles << Qt::CheckStateRole;
        emit dataChanged(index, index, roles);
        return true;
    }
    return QFileSystemModel::setData(index, value, role);
}

void CustomFileSystemModel::reset()
{
    _checklist.clear();
    _partialChecklist.clear();
}
