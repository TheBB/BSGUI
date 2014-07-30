#include <thread>
#include <QBrush>
#include <QFileInfo>
#include <QIcon>

#include <GoTools/geometry/ObjectHeader.h>
#include <GoTools/trivariate/SplineVolume.h>

#include "DisplayObjects/Volume.h"

#include "ObjectSet.h"


inline bool modeMatch(SelectionMode mode, ComponentType type)
{
    return (mode == SM_FACE && type == CT_FACE ||
            mode == SM_EDGE && type == CT_EDGE ||
            mode == SM_POINT && type == CT_POINT);
}


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
    return (_type == CT_FACE ? "Faces" : (_type == CT_EDGE ? "Edges" : "Vertices"));
}


Component::Component(uint index, Node *parent)
    : Node(parent)
    , _index(index)
{
}


QString Component::displayString()
{
    Components *p = static_cast<Components *>(_parent);
    return (QString("%1 %2")
            .arg(p->cType() == CT_FACE ? "Face" : (p->cType() == CT_EDGE ? "Edge" : "Vertex"))
            .arg(_index + 1));
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


void ObjectSet::setSelectionMode(SelectionMode mode, bool fromMouse)
{
    if (mode != _selectionMode)
    {
        _selectionMode = mode;

        for (auto p : selectedObjects)
            p->obj()->selectionMode(mode, true);

        for (auto f : root->children())
            for (auto p : f->children())
                signalCheckChange(static_cast<Patch *>(p));
    }

    emit selectionChanged();
    emit selectionModeChanged(_selectionMode, fromMouse);
}


void ObjectSet::showSelected(bool visible)
{
    for (auto p : selectedObjects)
    {
        p->obj()->showSelected(_selectionMode, visible);
        signalVisibleChange(p);
    }

    emit update();
}


void ObjectSet::showAllSelectedPatches(bool visible)
{
    for (auto p : selectedObjects)
    {
        p->obj()->showSelected(SM_PATCH, visible);
        signalVisibleChange(p);
    }

    emit update();
}


void ObjectSet::showAll()
{
    for (auto p : displayObjects)
    {
        p->obj()->showSelected(SM_PATCH, true);
        signalVisibleChange(p);
    }

    emit update();
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
    return 3;
}


QVariant ObjectSet::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0: return QString("Name");
        case 1: case 2: return QString("");
        }
    }

    if (orientation == Qt::Horizontal && role == Qt::SizeHintRole)
        return section > 0 ? QSize(24, 32) : QSize();

    return QVariant();
}


QVariant ObjectSet::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    Node *node = static_cast<Node *>(index.internalPointer());

    if (role == Qt::DisplayRole && index.column() == 0)
        return node->displayString();

    if (role == Qt::ForegroundRole && index.column() == 0)
    {
        ComponentType type;
        if (node->type() == NT_COMPONENTS)
            type = static_cast<Components *>(node)->cType();
        else if (node->type() == NT_COMPONENT)
            type = static_cast<Components *>(node->parent())->cType();
        else
            return QVariant();

        return QBrush(QColor(modeMatch(_selectionMode, type) ? "black" : "silver"));
    }

    if (role == Qt::CheckStateRole && index.column() == 2)
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
            return QVariant(obj->fullSelection(_selectionMode) ? Qt::Checked :
                            (obj->hasSelection() ? Qt::PartiallyChecked : Qt::Unchecked));
        }
        case NT_COMPONENT:
            if (modeMatch(_selectionMode, static_cast<Components *>(node->parent())->cType()))
                return static_cast<Component *>(node)->isSelected() ? Qt::Checked : Qt::Unchecked;
            return QVariant();
        }
    }

    if (role == Qt::DecorationRole && index.column() == 1)
    {
        if (node->type() == NT_PATCH)
        {
            DisplayObject *obj = static_cast<Patch *>(node)->obj();
            QString base = ":/icons/%1_%2.png";

            switch (obj->type())
            {
            case OT_VOLUME: base = base.arg("volume");
            }

            base = base.arg(obj->isFullyVisible(false) ? "full" :
                            (obj->isInvisible(false) ? "hidden" : "partial"));
            return QIcon(base);
        }
        else if (node->type() == NT_FILE)
        {
            bool allInvisible = true, allVisible = true;
            for (auto n : node->children())
            {
                DisplayObject *obj = static_cast<Patch *>(n)->obj();

                allInvisible &= obj->isInvisible(false);
                allVisible &= obj->isFullyVisible(false);

                if (!allInvisible && !allVisible)
                    return QIcon(":/icons/file_partial.png");
            }

            return QIcon(allVisible ? ":/icons/file_full.png" : ":/icons/file_hidden.png");
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

    Qt::ItemFlags flags = QAbstractItemModel::flags(index) & ~Qt::ItemIsSelectable;

    Node *node = static_cast<Node *>(index.internalPointer());

    switch (node->type())
    {
    case NT_FILE:
        return flags | Qt::ItemIsUserCheckable;
    case NT_PATCH:
        return flags | Qt::ItemIsUserCheckable;
    case NT_COMPONENTS:
        return flags & ~Qt::ItemIsSelectable;
    case NT_COMPONENT:
        return flags | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren;
    }
}


void ObjectSet::addPatchesFromFile(std::string fileName)
{
    std::ifstream file(fileName);
    if (!file.good())
    {
        std::cerr << "Couldn't open file '" << fileName << "'." << std::endl;
        return;
    }

    Go::ObjectHeader head;
    head.read(file);

    DisplayObject *obj = NULL;

    switch (head.classType())
    {
    case Go::Class_SplineVolume:
    {
        Go::SplineVolume *v = new Go::SplineVolume();
        v->read(file);
        obj = new Volume(v);
        break;
    }
    }

    file.close();

    if (!obj)
        return;

    while (!obj->initialized())
    {
        emit requestInitialization(obj);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!obj->initialized())
    {
        std::cerr << "Failed to initialized display object. (This should never happen.)";
        return;
    }

    m.lock();

    Node *fileNode = getOrCreateFileNode(QString(fileName.c_str()));

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
    m.lock();

    if (clear)
    {
        for (auto p : selectedObjects)
        {
            p->obj()->selectObject(_selectionMode, false);
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
        case SM_PATCH: (*it)->obj()->selectObject(SM_PATCH, true); break;
        case SM_FACE:  (*it)->obj()->selectFaces(true,  {p - (*it)->obj()->baseColor()}); break;
        case SM_EDGE:  (*it)->obj()->selectEdges(true,  {p - (*it)->obj()->baseColor()}); break;
        case SM_POINT: (*it)->obj()->selectPoints(true, {p - (*it)->obj()->baseColor()}); break;
        }

        changedPatches.insert(*it);
    }

    m.unlock();

    for (auto p : changedPatches)
        signalCheckChange(p);

    emit selectionChanged();
}


void ObjectSet::addToSelection(Node *node, bool signal)
{
    if (node->type() == NT_FILE)
    {
        for (auto n : node->children())
            addToSelection(n, false);
    }
    else if (node->type() == NT_PATCH)
    {
        Patch *patch = static_cast<Patch *>(node);

        m.lock();
        selectedObjects.insert(patch);
        m.unlock();

        patch->obj()->selectObject(_selectionMode, true);

        signalCheckChange(patch);
    }
    else if (node->type() == NT_COMPONENT)
    {
        ComponentType type = static_cast<Components *>(node->parent())->cType();

        if (modeMatch(_selectionMode, type))
        {
            Patch *patch = static_cast<Patch *>(node->parent()->parent());

            switch (type)
            {
            case CT_FACE:
                patch->obj()->selectFaces(true, {static_cast<Component *>(node)->index()});
                break;
            case CT_EDGE:
                patch->obj()->selectEdges(true, {static_cast<Component *>(node)->index()});
                break;
            case CT_POINT:
                patch->obj()->selectPoints(true, {static_cast<Component *>(node)->index()});
                break;
            }

            m.lock();
            selectedObjects.insert(patch);
            m.unlock();

            signalCheckChange(patch);
        }
    }

    if (signal)
        emit selectionChanged();
}


void ObjectSet::removeFromSelection(Node *node, bool signal)
{
    if (node->type() == NT_FILE)
    {
        for (auto n : node->children())
            removeFromSelection(n, false);
    }
    else if (node->type() == NT_PATCH)
    {
        Patch *patch = static_cast<Patch *>(node);
        patch->obj()->selectObject(_selectionMode, false);

        m.lock();
        selectedObjects.erase(patch);
        m.unlock();

        signalCheckChange(patch);
    }
    else if (node->type() == NT_COMPONENT)
    {
        ComponentType type = static_cast<Components *>(node->parent())->cType();
        if (modeMatch(_selectionMode, type))
        {
            Patch *patch = static_cast<Patch *>(node->parent()->parent());

            switch (type)
            {
            case CT_FACE:
                patch->obj()->selectFaces(false, {static_cast<Component *>(node)->index()});
                break;
            case CT_EDGE:
                patch->obj()->selectEdges(false, {static_cast<Component *>(node)->index()});
                break;
            case CT_POINT:
                patch->obj()->selectPoints(false, {static_cast<Component *>(node)->index()});
                break;
            }

            if (!patch->obj()->hasSelection())
            {
                m.lock();
                selectedObjects.erase(patch);
                m.unlock();
            }

            signalCheckChange(patch);
        }
    }

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
        if (c->obj()->isInvisible(false))
            continue;

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
        if (c->obj()->isInvisible(false))
            continue;

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
    {
        emit dataChanged(createIndex(0, 0, n->getChild(0)),
                         createIndex(n->nChildren()-1, 0, n->getChild(n->nChildren()-1)),
                         QVector<int>(Qt::ForegroundRole));
        emit dataChanged(createIndex(0, 2, n->getChild(0)),
                         createIndex(n->nChildren()-1, 2, n->getChild(n->nChildren()-1)),
                         QVector<int>(Qt::CheckStateRole));
    }

    emit dataChanged(createIndex(0, 0, patch->getChild(0)),
                     createIndex(patch->nChildren()-1, 0, patch->getChild(patch->nChildren()-1)),
                     QVector<int>(Qt::ForegroundRole));

    QModelIndex patchIndex = createIndex(patch->indexInParent(), 2, patch);
    emit dataChanged(patchIndex, patchIndex, QVector<int>(Qt::CheckStateRole));

    QModelIndex fileIndex = createIndex(patch->parent()->indexInParent(), 2, patch->parent());
    emit dataChanged(fileIndex, fileIndex, QVector<int>(Qt::CheckStateRole));
}


void ObjectSet::signalVisibleChange(Patch *patch)
{
    QModelIndex patchIndex = createIndex(patch->indexInParent(), 1, patch);
    emit dataChanged(patchIndex, patchIndex, QVector<int>(Qt::DecorationRole));

    QModelIndex fileIndex = createIndex(patch->parent()->indexInParent(), 1, patch->parent());
    emit dataChanged(fileIndex, fileIndex, QVector<int>(Qt::DecorationRole));
}
