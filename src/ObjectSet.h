#include <vector>

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QString>

#include "DispObject.h"


#ifndef _OBJECTSET_H_
#define _OBJECTSET_H_

typedef struct File
{
    QString fileName;

    File (QString fn) : fileName(fn) { }
} File;

typedef struct Node 
{
    enum NodeType { RootNode, FileNode, VolumeNode } type;

    union
    {
        void *root;
        File *file;
        DispObject *volume;
    } data;

    Node *parent;
    std::vector<Node *> children;

    Node(NodeType type, Node *parent = NULL, void *data = NULL)
    {
        this->type = type;

        this->parent = parent;
        if (parent)
            parent->children.push_back(this);

        switch (type)
        {
        case RootNode: this->data.root = NULL;
        case FileNode: this->data.file = static_cast<File *>(data);
        case VolumeNode: this->data.volume = static_cast<DispObject *>(data);
        }
    }

    ~Node()
    {
        switch (type)
        {
        case FileNode: delete data.file; break;
        case VolumeNode: delete data.volume; break;
        }

        for (auto child : children)
            delete child;
    }

    int indexOf(Node *child)
    {
        for (int i = 0; i < children.size(); i++)
            if (child == children[i])
                return i;
        return -1;
    }

} Node;

class ObjectSet : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ObjectSet(QObject *parent = NULL);
    ~ObjectSet();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    Node *root;
};

#endif /* _OBJECTSET_H_ */
