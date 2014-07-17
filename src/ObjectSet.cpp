#include "ObjectSet.h"


ObjectSet::ObjectSet(QObject *parent)
    : QAbstractItemModel(parent)
{
    root = new Node(Node::RootNode);
    Node *fileNodeA = new Node(Node::FileNode, root, new File("myfile.g2"));
    Node *volumeNodeA = new Node(Node::VolumeNode, fileNodeA, new DispObject());
    Node *volumeNodeB = new Node(Node::VolumeNode, fileNodeA, new DispObject());
    Node *volumeNodeC = new Node(Node::VolumeNode, fileNodeA, new DispObject());
    Node *fileNodeB = new Node(Node::FileNode, root, new File("anotherfile.g2"));
    Node *volumeNodeD = new Node(Node::VolumeNode, fileNodeB, new DispObject());
    Node *volumeNodeE = new Node(Node::VolumeNode, fileNodeB, new DispObject());
    Node *volumeNodeF = new Node(Node::VolumeNode, fileNodeB, new DispObject());
    Node *volumeNodeG = new Node(Node::VolumeNode, fileNodeB, new DispObject());
    Node *volumeNodeH = new Node(Node::VolumeNode, fileNodeB, new DispObject());
}


ObjectSet::~ObjectSet()
{
    delete root;
}


QModelIndex ObjectSet::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Node *parentNode = parent.isValid() ? static_cast<Node *>(parent.internalPointer()) : root;
    return createIndex(row, column, parentNode->children[row]);
}


QModelIndex ObjectSet::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Node *parentNode = static_cast<Node *>(index.internalPointer())->parent;

    if (parentNode == root)
        return QModelIndex();

    return createIndex(parentNode->parent->indexOf(parentNode), 0, parentNode);
}


int ObjectSet::rowCount(const QModelIndex &index) const
{
    if (index.column() > 0)
        return 0;

    Node *parentNode = index.isValid() ? static_cast<Node *>(index.internalPointer()) : root;
    return parentNode->children.size();
}


int ObjectSet::columnCount(const QModelIndex &index) const
{
    return 1;
}


QVariant ObjectSet::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    Node *node = static_cast<Node *>(index.internalPointer());

    switch (node->type)
    {
    case Node::RootNode: return QString("BREAKAGE");
    case Node::FileNode: return node->data.file->fileName;
    case Node::VolumeNode: return QString("A patch");
    }
}


Qt::ItemFlags ObjectSet::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}
