#include <QFileInfo>

#include <GoTools/geometry/ObjectHeader.h>

#include "GLWidget.h"

#include "ObjectSet.h"


Node::Node(Node *parent)
{
    _parent = parent;
    if (parent)
        parent->addChild(this);
}


Node::~Node()
{
    for (auto c : _children)
        delete c;
}


void Node::addChild(Node *child)
{
    _children.push_back(child);
}


Node *Node::getChild(int idx)
{
    return _children[idx];
}


int Node::indexOfChild(Node *child)
{
    for (int i = 0; i < _children.size(); i++)
        if (_children[i] == child)
            return i;
    return -1;
}


int Node::numberOfChildren()
{
    return _children.size();
}


File::File(QString fn, Node *parent)
    : Node(parent)
{
    if (fn == "")
    {
        fileName = "<none>";
        absolutePath = "";
    }
    else
    {
        QFileInfo info(fn);
        fileName = info.fileName();
        absolutePath = info.absoluteFilePath();
    }
}


QString File::displayString()
{
    return fileName;
}


Patch::Patch(DispObject *obj, Node *parent)
    : Node(parent)
{
    _obj = obj;
}


QString Patch::displayString()
{
    return QString("Patch %1").arg(_parent->indexOfChild(this) + 1);
}


ObjectSet::ObjectSet(QObject *parent)
    : QAbstractItemModel(parent)
{
    root = new Node();
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
    return createIndex(row, column, parentNode->getChild(row));
}


QModelIndex ObjectSet::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Node *parentNode = static_cast<Node *>(index.internalPointer())->parent();

    if (parentNode == root)
        return QModelIndex();

    return createIndex(parentNode->parent()->indexOfChild(parentNode), 0, parentNode);
}


int ObjectSet::rowCount(const QModelIndex &index) const
{
    if (index.column() > 0)
        return 0;

    Node *parentNode = index.isValid() ? static_cast<Node *>(index.internalPointer()) : root;
    return parentNode->numberOfChildren();
}


int ObjectSet::columnCount(const QModelIndex &index) const
{
    return 1;
}


QVariant ObjectSet::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
        return QString("Name");
    return QVariant();
}


QVariant ObjectSet::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    Node *node = static_cast<Node *>(index.internalPointer());
    return node->displayString();
}


Qt::ItemFlags ObjectSet::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}


void ObjectSet::addCubeFromCenter(QVector3D center)
{
    DispObject *obj = new DispObject(center);
    emit requestInitialization(obj);

    if (!obj->initialized())
    {
        qDebug() << "Failed to initialize display object!";
        delete obj;
        return;
    }

    m.lock();

    Node *fileNode = getFileNode("");
    if (!fileNode)
        fileNode = new File("", root);
    new Patch(obj, fileNode);
    dispObjects.insert(obj);

    m.unlock();
}


Node *ObjectSet::getFileNode(QString fileName)
{
    for (auto node : root->children())
    {
        if (!node->type() == NT_FILE)
            continue;
        if (static_cast<File *>(node)->matches(fileName))
            return node;
    }

    return NULL;
}
