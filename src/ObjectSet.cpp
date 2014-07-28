#include <thread>
#include <QFileInfo>

#include <GoTools/geometry/ObjectHeader.h>

#include "GLWidget.h"
#include "DisplayObjects/Volume.h"

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
        case NT_COMPONENTS: delete static_cast<Components *>(c); break;
        case NT_COMPONENT: delete static_cast<Component *>(c); break;
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


int Node::nChildren()
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


Patch::Patch(DisplayObject *obj, Node *parent)
    : Node(parent)
    , _obj(obj)
{
    if (obj->nFaces() > 0)
    {
        Components *faces = new Components(CT_FACE, this);
        for (int i = 0; i < obj->nFaces(); i++)
            new Component(i, faces);
    }

    if (obj->nEdges() > 0)
    {
        Components *edges = new Components(CT_EDGE, this);
        for (int i = 0; i < obj->nEdges(); i++)
            new Component(i, edges);
    }

    if (obj->nPoints() > 0)
    {
        Components *points = new Components(CT_POINT, this);
        for (int i = 0; i < obj->nPoints(); i++)
            new Component(i, points);
    }
}


Patch::~Patch()
{
    delete _obj;
}


QString Patch::displayString()
{
    return QString("Patch %1").arg(indexInParent() + 1);
}


Components::Components(ComponentType type, Node *parent)
    : Node(parent)
    , _type(type)
{
}


QString Components::displayString()
{
    return (_type == CT_FACE ? "Faces" : (_type == CT_EDGE ? "Edges" : "Points"));
}


Component::Component(int index, Node *parent)
    : Node(parent)
    , _index(index)
{
}


QString Component::displayString()
{
    Components *p = static_cast<Components *>(_parent);
    return QString("%1 %2")
        .arg(p->cType() == CT_FACE ? "Face" : (p->cType() == CT_EDGE ? "Edge" : "Point"))
        .arg(_index + 1);
}


bool Component::isSelected()
{
    DisplayObject *obj = static_cast<Patch *>(_parent->parent())->obj();

    switch (static_cast<Components *>(_parent)->cType())
    {
    case CT_FACE: return obj->faceSelected(_index);
    case CT_EDGE: return obj->edgeSelected(_index);
    case CT_POINT: return obj->pointSelected(_index);
    }
}


ObjectSet::ObjectSet(QObject *parent)
    : QAbstractItemModel(parent)
    , _selectionMode(SM_PATCH)
{
    root = new Node();
}


ObjectSet::~ObjectSet()
{
    delete root;
}


void ObjectSet::setSelectionMode(SelectionMode mode)
{
    if (mode != _selectionMode)
    {
        _selectionMode = mode;

        for (auto p : selectedObjects)
            p->obj()->selectionMode(mode, true);
    }

    emit selectionChanged();
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
    return parentNode->nChildren();
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
                DisplayObject *obj = static_cast<Patch *>(n)->obj();

                foundUnselected |= !obj->fullSelection(_selectionMode);
                foundSelected |= obj->hasSelection();

                if (foundUnselected && foundSelected)
                    return Qt::PartiallyChecked;
            }

            return foundSelected ? Qt::Checked : Qt::Unchecked;
        }
        case NT_PATCH:
        {
            DisplayObject *obj = static_cast<Patch *>(node)->obj();
            return QVariant(obj->fullSelection(_selectionMode)
                            ? Qt::Checked
                            : (obj->hasSelection()
                               ? Qt::PartiallyChecked
                               : Qt::Unchecked));
        }
        case NT_COMPONENT:
            return static_cast<Component *>(node)->isSelected() ? Qt::Checked : Qt::Unchecked;
        }
    }

    return QVariant();
}


bool ObjectSet::setData(const QModelIndex &index, const QVariant &value, int role)
{
    // if (role == Qt::CheckStateRole)
    // {
    //     if (value.toInt() == Qt::Checked)
    //         addToSelection(static_cast<Node *>(index.internalPointer()));
    //     else
    //         removeFromSelection(static_cast<Node *>(index.internalPointer()));
    // }

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
    case NT_COMPONENTS:
        return flags;
    case NT_COMPONENT:
        return flags | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren;
    }
}


void ObjectSet::addCubeFromCenter(QVector3D center)
{
    DisplayObject *obj = new Volume(center);
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
    beginInsertRows(index, fileNode->nChildren(), fileNode->nChildren());
    Patch *patch = new Patch(obj, fileNode);
    endInsertRows();

    displayObjects.push_back(patch);

    m.unlock();

    emit update();
}


void ObjectSet::boundingSphere(QVector3D *center, float *radius)
{
    if (displayObjects.empty())
    {
        *center = QVector3D(0,0,0);
        *radius = 0.0;
        return;
    }

    m.lock();

    std::vector<Patch *> *vec = &displayObjects;
    if (!selectedObjects.empty())
        vec = new std::vector<Patch *>(selectedObjects.begin(), selectedObjects.end());

    DisplayObject *a = (*vec)[0]->obj(), *b;
    farthestPointFrom(a, &b, vec);
    farthestPointFrom(b, &a, vec);

    *center = (a->center() + b->center()) / 2;
    *radius = (a->center() - b->center()).length() / 2;

    ritterSphere(center, radius, vec);

    float maxRadius = 0.0;
    for (auto c : displayObjects)
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
        for (auto p : selectedObjects)
        {
            p->obj()->selectObject(false);
            signalCheckChange(p);
        }

        selectedObjects.clear();
    }

    std::set<Patch *> changedPatches;

    std::vector<Patch *>::iterator it = displayObjects.begin();
    for (auto p : *picks)
    {
        while (!(*it)->obj()->hasColor(p) && it != displayObjects.end())
            it++;

        if (it == displayObjects.end())
            break;

        selectedObjects.insert(*it);

        switch (_selectionMode)
        {
        case SM_PATCH: (*it)->obj()->selectObject(true); break;
        case SM_FACE:  (*it)->obj()->selectFaces(true,  {p - (*it)->obj()->baseColor()}); break;
        case SM_EDGE:  (*it)->obj()->selectEdges(true,  {p - (*it)->obj()->baseColor()}); break;
        case SM_POINT: (*it)->obj()->selectPoints(true, {p - (*it)->obj()->baseColor()}); break;
        }

        changedPatches.insert(*it);
    }

    for (auto p : changedPatches)
        signalCheckChange(p);

    emit selectionChanged();
}


// void ObjectSet::addToSelection(Node *node, bool signal)
// {
//     if (node->type() == NT_FACE)
//     {
//         Patch *patch = static_cast<Patch *>(node->parent());
//         if (!_selectFaces)
//         {
//             addToSelection(patch, true);
//             return;
//         }
//         selectedObjects.insert(patch);
//         patch->obj()->selectFace(node->indexInParent());

//         signalCheckChange(patch);
//     }
//     else if (node->type() == NT_PATCH)
//     {
//         Patch *patch = static_cast<Patch *>(node);
//         selectedObjects.insert(patch);
//         for (int i = 0; i < 6; i++)
//             patch->obj()->selectFace(i);

//         signalCheckChange(patch);
//     }
//     else if (node->type() == NT_FILE)
//         for (auto n : node->children())
//             addToSelection(n, false);


//     if (signal)
//         emit selectionChanged();
// }


// void ObjectSet::removeFromSelection(Node *node, bool signal)
// {
//     if (node->type() == NT_FACE)
//     {
//         Patch *patch = static_cast<Patch *>(node->parent());
//         if (!_selectFaces)
//         {
//             removeFromSelection(patch, true);
//             return;
//         }
//         patch->obj()->selectFace(node->indexInParent(), false);
//         if (!patch->obj()->hasSelection())
//             selectedObjects.erase(patch);

//         signalCheckChange(patch);
//     }
//     else if (node->type() == NT_PATCH)
//     {
//         Patch *patch = static_cast<Patch *>(node);
//         patch->obj()->clearSelection();
//         selectedObjects.erase(patch);

//         signalCheckChange(patch);
//     }
//     else if (node->type() == NT_FILE)
//         for (auto n : node->children())
//             removeFromSelection(n, false);

//     if (signal)
//         emit selectionChanged();
// }


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
        int row = root->nChildren();
        beginInsertRows(QModelIndex(), row, row);
        node = new File(fileName, root);
        endInsertRows();
    }

    return node;
}


void ObjectSet::farthestPointFrom(DisplayObject *a, DisplayObject **b, std::vector<Patch *> *vec)
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
    for (Node *n : patch->children())
        emit dataChanged(createIndex(0, 0, n->getChild(0)),
                         createIndex(n->nChildren()-1, 0, n->getChild(n->nChildren()-1)),
                         QVector<int>(Qt::CheckStateRole));

    emit dataChanged(createIndex(0, 0, patch->getChild(0)),
                     createIndex(patch->nChildren()-1, 0, patch->getChild(patch->nChildren()-1)),
                     QVector<int>(Qt::CheckStateRole));

    QModelIndex patchIndex = createIndex(patch->indexInParent(), 0, patch);
    emit dataChanged(patchIndex, patchIndex, QVector<int>(Qt::CheckStateRole));

    QModelIndex fileIndex = createIndex(patch->parent()->indexInParent(), 0, patch->parent());
    emit dataChanged(fileIndex, fileIndex, QVector<int>(Qt::CheckStateRole));
}
