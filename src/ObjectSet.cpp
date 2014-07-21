#include <thread>
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
    {
        switch (c->type())
        {
        case NT_ROOT: delete static_cast<Node *>(c); break;
        case NT_FILE: delete static_cast<File *>(c); break;
        case NT_PATCH: delete static_cast<Patch *>(c); break;
        case NT_FACE: delete static_cast<Face *>(c); break;
        }
    }
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


int Node::indexInParent()
{
    return _parent->indexOfChild(this);
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
    , _obj(obj)
{
}


Patch::~Patch()
{
    delete _obj;
}


QString Patch::displayString()
{
    return QString("Patch %1").arg(indexInParent() + 1);
}


Face::Face(int index, Node *parent)
    : Node(parent)
    , _index(index)
{
}


QString Face::displayString()
{
    return QString("Face %1").arg(_index + 1);
}


ObjectSet::ObjectSet(QObject *parent)
    : QAbstractItemModel(parent)
    , _selectFaces(false)
{
    root = new Node();
}


ObjectSet::~ObjectSet()
{
    delete root;
}


void ObjectSet::setSelectFaces(bool val, bool fromMouse)
{
    _selectFaces = val;

    if (!_selectFaces)
    {
        for (auto p : selectedObjects)
        {
            for (int i = 0; i < 6; i++)
                p->obj()->selectFace(i);
            signalCheckChange(p);
        }

        emit selectionChanged();
    }

    emit selectFacesChanged(val, fromMouse);
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

    return createIndex(parentNode->indexInParent(), 0, parentNode);
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

    Node *node = static_cast<Node *>(index.internalPointer());

    if (role == Qt::DisplayRole)
        return node->displayString();

    if (role == Qt::CheckStateRole)
    {
        switch (node->type())
        {
        case NT_FILE:
        {
            bool foundUnselected = false, foundSelected = false;
            for (auto n : node->children())
            {
                DispObject *obj = static_cast<Patch *>(n)->obj();

                foundUnselected |= !obj->fullSelection();
                foundSelected |= obj->hasSelection();

                if (foundUnselected && foundSelected)
                    return Qt::PartiallyChecked;
            }

            return foundSelected ? Qt::Checked : Qt::Unchecked;
        }
        case NT_PATCH:
        {
            DispObject *obj = static_cast<Patch *>(node)->obj();
            return QVariant(
                obj->fullSelection()
                ? Qt::Checked
                : (obj->hasSelection()
                   ? Qt::PartiallyChecked
                   : Qt::Unchecked));
        }
        case NT_FACE:
            return QVariant(
                static_cast<Patch *>(node->parent())->obj()->isFaceSelected(node->indexInParent())
                ? Qt::Checked : Qt::Unchecked);
        }
    }

    return QVariant();
}


bool ObjectSet::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole)
    {
        if (value.toInt() == Qt::Checked)
            addToSelection(static_cast<Node *>(index.internalPointer()));
        else
            removeFromSelection(static_cast<Node *>(index.internalPointer()));
    }

    return false;
}


Qt::ItemFlags ObjectSet::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    Node *node = static_cast<Node *>(index.internalPointer());

    switch (node->type())
    {
    case NT_FILE:
        return flags | Qt::ItemIsUserCheckable;
    case NT_PATCH:
        return flags | Qt::ItemIsUserCheckable;
    case NT_FACE:
        return flags | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren;
    }
}


void ObjectSet::addCubeFromCenter(QVector3D center)
{
    DispObject *obj = new DispObject(center);
    emit requestInitialization(obj);

    while (!obj->initialized())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    if (!obj->initialized())
    {
        qDebug() << "Failed to initialize display object! (This should never happen.)";
        return;
    }

    m.lock();

    Node *fileNode = getOrCreateFileNode("");

    QModelIndex index = createIndex(fileNode->indexInParent(), 0, fileNode);
    beginInsertRows(index, fileNode->numberOfChildren(), fileNode->numberOfChildren());
    Patch *patch = new Patch(obj, fileNode);
    endInsertRows();

    index = createIndex(patch->indexInParent(), 0, patch);
    beginInsertRows(index, 0, 5);
    for (int i = 0; i < 6; i++)
        new Face(i, patch);
    endInsertRows();

    dispObjects.push_back(patch);

    m.unlock();

    emit update();
}


void ObjectSet::boundingSphere(QVector3D *center, float *radius)
{
    if (dispObjects.empty())
    {
        *center = QVector3D(0,0,0);
        *radius = 0.0;
        return;
    }

    m.lock();

    std::vector<Patch *> *vec;

    if (selectedObjects.empty())
        vec = &dispObjects;
    else
        vec = new std::vector<Patch *>(selectedObjects.begin(), selectedObjects.end());

    DispObject *a = (*vec)[0]->obj(), *b;
    farthestPointFrom(a, &b, vec);
    farthestPointFrom(b, &a, vec);

    *center = (a->center() + b->center()) / 2;
    *radius = (a->center() - b->center()).length() / 2;

    ritterSphere(center, radius, vec);

    float maxRadius = 0.0;
    for (auto c : dispObjects)
        if (c->obj()->radius() > maxRadius)
            maxRadius = c->obj()->radius();

    *radius += 2 * maxRadius;

    if (!selectedObjects.empty())
        delete vec;

    m.unlock();
}


void ObjectSet::setSelection(std::set<uint> *picks, bool clear)
{
    if (clear)
    {
        for (auto o : selectedObjects)
        {
            o->obj()->clearSelection();
            signalCheckChange(static_cast<Patch *>(o));
        }
        selectedObjects.clear();
    }

    for (auto p : *picks)
    {
        int idx = p / 6;
        int face = p % 6;

        if (idx > dispObjects.size() || idx < 0 || face > 5 || face < 0)
            continue;

        selectedObjects.insert(dispObjects[idx]);

        if (_selectFaces)
            dispObjects[idx]->obj()->selectFace(face);
        else
            for (int i = 0; i < 6; i++)
                dispObjects[idx]->obj()->selectFace(i);

        signalCheckChange(dispObjects[idx]);
    }

    emit selectionChanged();
}


void ObjectSet::addToSelection(Node *node, bool signal)
{
    if (node->type() == NT_FACE)
    {
        Patch *patch = static_cast<Patch *>(node->parent());
        if (!_selectFaces)
        {
            addToSelection(patch, true);
            return;
        }
        selectedObjects.insert(patch);
        patch->obj()->selectFace(node->indexInParent());

        signalCheckChange(patch);
    }
    else if (node->type() == NT_PATCH)
    {
        Patch *patch = static_cast<Patch *>(node);
        selectedObjects.insert(patch);
        for (int i = 0; i < 6; i++)
            patch->obj()->selectFace(i);

        signalCheckChange(patch);
    }
    else if (node->type() == NT_FILE)
        for (auto n : node->children())
            addToSelection(n, false);


    if (signal)
        emit selectionChanged();
}


void ObjectSet::removeFromSelection(Node *node, bool signal)
{
    if (node->type() == NT_FACE)
    {
        Patch *patch = static_cast<Patch *>(node->parent());
        if (!_selectFaces)
        {
            removeFromSelection(patch, true);
            return;
        }
        patch->obj()->selectFace(node->indexInParent(), false);
        if (!patch->obj()->hasSelection())
            selectedObjects.erase(patch);

        signalCheckChange(patch);
    }
    else if (node->type() == NT_PATCH)
    {
        Patch *patch = static_cast<Patch *>(node);
        patch->obj()->clearSelection();
        selectedObjects.erase(patch);

        signalCheckChange(patch);
    }
    else if (node->type() == NT_FILE)
        for (auto n : node->children())
            removeFromSelection(n, false);

    if (signal)
        emit selectionChanged();
}


Node *ObjectSet::getOrCreateFileNode(QString fileName)
{
    Node *node = NULL;

    for (auto searchNode : root->children())
    {
        if (!searchNode->type() == NT_FILE)
            continue;
        if (static_cast<File *>(searchNode)->matches(fileName))
            node = searchNode;
    }

    if (!node)
    {
        int row = root->numberOfChildren();
        beginInsertRows(QModelIndex(), row, row);
        node = new File(fileName, root);
        endInsertRows();
    }

    return node;
}


void ObjectSet::farthestPointFrom(DispObject *a, DispObject **b, std::vector<Patch *> *vec)
{
    float distance = -1;

    for (auto c : *vec)
    {
        float _distance = (c->obj()->center() - a->center()).length();
        if (_distance > distance)
        {
            distance = _distance;
            *b = c->obj();
        }
    }
}


void ObjectSet::ritterSphere(QVector3D *center, float *radius, std::vector<Patch *> *vec)
{
    for (auto c : *vec)
    {
        float d = (c->obj()->center() - (*center)).length();
        if (d > (*radius))
        {
            *center = ((d + (*radius))/2 * (*center) + (d - (*radius))/2 * c->obj()->center()) / d;
            *radius = (d + (*radius))/2;
        }
    }
}


void ObjectSet::signalCheckChange(Patch *patch)
{
    emit dataChanged(createIndex(0, 0, patch->getChild(0)),
                     createIndex(5, 0, patch->getChild(5)),
                     QVector<int>(Qt::CheckStateRole));

    QModelIndex patchIndex = createIndex(patch->indexInParent(), 0, patch);
    emit dataChanged(patchIndex, patchIndex, QVector<int>(Qt::CheckStateRole));

    QModelIndex fileIndex = createIndex(patch->parent()->indexInParent(), 0, patch->parent());
    emit dataChanged(fileIndex, fileIndex, QVector<int>(Qt::CheckStateRole));
}
