#include <algorithm>
#include <thread>
#include <QBrush>
#include <QFileInfo>
#include <QIcon>

#include <GoTools/geometry/ObjectHeader.h>
#include <GoTools/trivariate/SplineVolume.h>
#include <GoTools/geometry/SplineSurface.h>

#include "DisplayObjects/Volume.h"
#include "DisplayObjects/Surface.h"
#include "DisplayObjects/Curve.h"

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
    , _change(FC_NONE)
    , lastCheckedSize(0)
{
    m.lock();

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
        _size = info.size();
        modified = info.lastModified();

        computeChecksums(&checksums);
    }

    m.unlock();
}


void File::refreshInfo()
{
    QFileInfo info(absolutePath);
    _size = info.size();
    modified = info.lastModified();

    computeChecksums(&checksums);
}


QString File::displayString()
{
    return fileName;
}


void File::checkChange()
{
    QFileInfo info(absolutePath);

    if (!info.exists())
        _change = FC_DELETED;
    else if (info.size() != _size || info.lastModified() > modified)
    {
        if (info.size() == lastCheckedSize)
            _change = FC_CHANGED;
        else
            _change = FC_CHANGING;
        lastCheckedSize = info.size();
    }
    else
        _change = FC_NONE;
}


void File::clearPatches()
{
    while (!_children.empty())
    {
        delete static_cast<Patch *>(_children.back());
        _children.pop_back();
    }
}


void File::computeChecksums(std::vector<size_t> *ret)
{
    ret->clear();

    std::ifstream stream(absolutePath.toStdString());
    if (!stream.good())
        return;

    size_t hash = 0;
    std::hash<std::string> hasher;
    std::string s;
    bool hasHash = false;
    while (std::getline(stream, s))
    {
        if (s == "")
        {
            if (hasHash)
            {
                ret->push_back(hash);
                hash = 0;
                hasHash = false;
            }
        }
        else
        {
            hash += hasher(s);
            hasHash = true;
        }
    }

    stream.close();
}


Patch::Patch(DisplayObject *obj, Node *parent)
    : Node(parent)
    , _obj(obj)
{
    obj->setPatch(this);

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
    , watch(true)
{
    root = new Node();

    fileWatcher = std::thread([this] () { watchFiles(); });
}


ObjectSet::~ObjectSet()
{
    watch = false;
    fileWatcher.join();

    delete root;
}


void ObjectSet::loadFile(QString fileName)
{
    mQueue.lock();
    loadQueue.insert(fileName);
    mQueue.unlock();

    emit log(QString("Queued '%1' for loading").arg(QFileInfo(fileName).fileName()), LL_NORMAL);
}


void ObjectSet::watchFiles()
{
    int n = 0;

    while (watch)
    {
        mQueue.lock();
        for (auto fn : loadQueue)
        {
            addPatchesFromFile(fn);
            if (!watch)
                break;
        }
        loadQueue.clear();
        mQueue.unlock();

        if (!watch)
            break;

        m.lock();
        for (auto f : root->children())
        {
            File *file = static_cast<File *>(f);

            FileChange old = file->change();
            file->checkChange();

            if (file->change() == FC_DELETED && old != FC_DELETED)
                emit log(QString("File '%1' was deleted, but the patches are still in memory")
                         .arg(file->fn()), LL_WARNING);
            else if (file->change() == FC_NONE && old == FC_DELETED)
                emit log(QString("File '%1' was restored, but is unchanged").arg(file->fn()), LL_WARNING);
            else if (file->change() == FC_CHANGED && old != FC_CHANGED)
            {
                emit log(QString("File '%1' has changed, queueing for reload").arg(file->fn()), LL_WARNING);

                mQueue.lock();
                loadQueue.insert(file->absolute());
                mQueue.unlock();
            }
        }
        m.unlock();

        if (watch)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}


bool ObjectSet::hasSelection()
{
    DisplayObject::m.lock();

    bool ret = any_of(DisplayObject::begin(), DisplayObject::end(),
                      [] (std::pair<const uint, DisplayObject *> &i) {
                          return i.second->hasSelection();
                      });

    DisplayObject::m.unlock();

    return ret;
}


void ObjectSet::setSelectionMode(SelectionMode mode)
{
    if (mode != _selectionMode)
    {
        std::lock(m, DisplayObject::m);

        _selectionMode = mode;

        for (auto i = DisplayObject::begin(); i != DisplayObject::end(); i++)
            i->second->selectionMode(mode, true);

        for (auto f : root->children())
            for (auto p : f->children())
                signalCheckChange(static_cast<Patch *>(p));

        m.unlock();
        DisplayObject::m.unlock();
    }

    emit selectionChanged();
    emit selectionModeChanged(_selectionMode);
}


void ObjectSet::showSelected(bool visible)
{
    std::lock(m, DisplayObject::m);

    for (auto i = DisplayObject::begin(); i != DisplayObject::end(); i++)
        if (i->second->hasSelection())
        {
            i->second->showSelected(_selectionMode, visible);
            signalVisibleChange(i->second->patch());
        }
    
    m.unlock();
    DisplayObject::m.unlock();

    emit update();
}


void ObjectSet::showAllSelectedPatches(bool visible)
{
    std::lock(m, DisplayObject::m);

    for (auto i = DisplayObject::begin(); i != DisplayObject::end(); i++)
        if (i->second->hasSelection())
        {
            i->second->showSelected(SM_PATCH, visible);
            signalVisibleChange(i->second->patch());
        }

    m.unlock();
    DisplayObject::m.unlock();

    emit update();
}


void ObjectSet::showAll()
{
    std::lock(m, DisplayObject::m);

    for (auto i = DisplayObject::begin(); i != DisplayObject::end(); i++)
    {
        i->second->showSelected(SM_PATCH, true);
        signalVisibleChange(i->second->patch());
    }

    m.unlock();
    DisplayObject::m.unlock();

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
            case OT_VOLUME: base = base.arg("volume"); break;
            case OT_SURFACE: base = base.arg("surface"); break;
            case OT_CURVE: base = base.arg("curve"); break;
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


void ObjectSet::addPatchesFromFile(QString fileName)
{
    File *file = getOrCreateFileNode(fileName);

    file->m.lock();

    file->refreshInfo();

    if (file->nChildren() > 0)
    {
        std::lock(m, DisplayObject::m);

        beginRemoveRows(createIndex(file->indexInParent(), 0, file), 0, file->nChildren() - 1);
        file->clearPatches();
        endRemoveRows();

        m.unlock();
        DisplayObject::m.unlock();
    }

    std::ifstream stream(file->absolute().toStdString());
    if (!stream.good())
    {
        emit log(QString("Failed to open file '%1'").arg(fileName), LL_ERROR);
        return;
    }

    emit log(QString("Opened file '%1' (%2 patches, %3 bytes)")
             .arg(file->fn())
             .arg(file->nChecksums())
             .arg(file->size()));

    bool cont = true;
    while (!stream.eof() && cont)
    {
        cont = addPatchFromStream(stream, file);
        cont &= watch;
        std::ws(stream);
    }

    file->m.unlock();

    stream.close();

    emit log(QString("Closed file '%1' (read %2 patches)")
             .arg(file->fn())
             .arg(file->nChildren()));
}


bool ObjectSet::addPatchFromStream(std::ifstream &stream, File *file)
{
    Go::ObjectHeader head;

    QString error = QString("%2 in '%1'").arg(file->fn());
    QString logString;

    try { head.read(stream); }
    catch (...)
    {
        emit log(error.arg("Unrecognized object header"), LL_ERROR);
        return false;
    }

    DisplayObject *obj = NULL;

    DisplayObject::m.lock();

    switch (head.classType())
    {
    case Go::Class_SplineVolume:
    {
        Go::SplineVolume *v = new Go::SplineVolume();
        try { v->read(stream); }
        catch (...)
        {
            emit log(error.arg("Unable to parse SplineVolume"), LL_ERROR);
            delete v;
            return false;
        }
        obj = new Volume(v);
        break;
    }
    case Go::Class_SplineSurface:
    {
        Go::SplineSurface *s = new Go::SplineSurface();
        try { s->read(stream); }
        catch (...)
        {
            emit log(error.arg("Unable to parse SplineSurface"), LL_ERROR);
            delete s;
            return false;
        }
        obj = new Surface(s);
        break;
    }
    case Go::Class_SplineCurve:
    {
        Go::SplineCurve *c = new Go::SplineCurve();
        try { c->read(stream); }
        catch (...)
        {
            emit log(error.arg("Unable to parse SplineCurve"), LL_ERROR);
            delete c;
            return false;
        }
        obj = new Curve(c);
        break;
    }
    default:
        emit log(error.arg(QString("Unrecognized class type %1").arg(head.classType())), LL_ERROR);
    }

    if (!obj)
    {
        DisplayObject::m.unlock();
        return false;
    }

    m.lock();
    QModelIndex index = createIndex(file->indexInParent(), 0, file);
    beginInsertRows(index, file->nChildren(), file->nChildren());
    Patch *patch = new Patch(obj, file);
    endInsertRows();
    m.unlock();

    DisplayObject::m.unlock();

    while (!obj->initialized())
    {
        emit requestInitialization(obj);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!obj->initialized())
    {
        emit log("Failed to initialize display object", LL_ERROR);
        delete obj;
        return true; // Can continue
    }

    emit update();

    return true;
}


void ObjectSet::boundingSphere(QVector3D *center, float *radius)
{
    DisplayObject::m.lock();

    if (DisplayObject::begin() == DisplayObject::end())
    {
        *center = QVector3D(0,0,0);
        *radius = 0.0;
        DisplayObject::m.unlock();
        return;
    }

    bool hasSelection = any_of(DisplayObject::begin(), DisplayObject::end(),
                               [] (std::pair<const uint, DisplayObject *> &i) {
                                   return i.second->hasSelection();
                               });

    DisplayObject *a = DisplayObject::begin()->second, *b;
    farthestPointFrom(a, &b, hasSelection);
    farthestPointFrom(b, &a, hasSelection);

    *center = (a->center() + b->center()) / 2;
    *radius = (a->center() - b->center()).length() / 2;

    ritterSphere(center, radius, hasSelection);

    float maxRadius = 0.0;
    for (auto i = DisplayObject::begin(); i != DisplayObject::end(); i++)
        if (i->second->radius() > maxRadius && (!hasSelection || i->second->hasSelection()))
            maxRadius = i->second->radius();

    *radius += 2 * maxRadius;

    DisplayObject::m.unlock();
}


void ObjectSet::setSelection(std::set<std::pair<uint,uint>> *picks, bool clear)
{
    std::lock(m, DisplayObject::m);

    if (clear)
        for (auto i = DisplayObject::begin(); i != DisplayObject::end(); i++)
            if (i->second->hasSelection())
            {
                i->second->selectObject(_selectionMode, false);
                signalCheckChange(i->second->patch());
            }

    std::set<Patch *> changedPatches;

    for (auto p : *picks)
    {
        DisplayObject *obj = DisplayObject::getObject(p.first);
        if (!obj)
            continue;

        if (obj->patch())
            changedPatches.insert(obj->patch());

        switch (_selectionMode)
        {
        case SM_PATCH: obj->selectObject(SM_PATCH, true); break;
        case SM_FACE: obj->selectFaces(true, {p.second}); break;
        case SM_EDGE: obj->selectEdges(true, {p.second}); break;
        case SM_POINT: obj->selectPoints(true, {p.second}); break;
        }
    }

    for (auto p : changedPatches)
        signalCheckChange(p);

    m.unlock();
    DisplayObject::m.unlock();

    emit selectionChanged();
}


void ObjectSet::addToSelection(Node *node, bool signal, bool lock)
{
    if (lock)
        std::lock(m, DisplayObject::m);

    if (node->type() == NT_FILE)
    {
        for (auto n : node->children())
            addToSelection(n, false, false);
    }
    else if (node->type() == NT_PATCH)
    {
        Patch *patch = static_cast<Patch *>(node);
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

            signalCheckChange(patch);
        }
    }

    if (lock)
    {
        m.unlock();
        DisplayObject::m.unlock();
    }

    if (signal)
        emit selectionChanged();
}


void ObjectSet::removeFromSelection(Node *node, bool signal, bool lock)
{
    if (lock)
        std::lock(m, DisplayObject::m);

    if (node->type() == NT_FILE)
    {
        for (auto n : node->children())
            removeFromSelection(n, false, false);
    }
    else if (node->type() == NT_PATCH)
    {
        Patch *patch = static_cast<Patch *>(node);
        patch->obj()->selectObject(_selectionMode, false);

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

            signalCheckChange(patch);
        }
    }

    if (lock)
    {
        m.unlock();
        DisplayObject::m.unlock();
    }

    if (signal)
        emit selectionChanged();
}


File *ObjectSet::getOrCreateFileNode(QString fileName)
{
    File *node = NULL;

    for (auto searchNode : root->children())
    {
        if (static_cast<File *>(searchNode)->matches(fileName))
        {
            node = static_cast<File *>(searchNode);
            break;
        }
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


void ObjectSet::farthestPointFrom(DisplayObject *a, DisplayObject **b, bool hasSelection)
{
    float distance = -1;

    for (auto i = DisplayObject::begin(); i != DisplayObject::end(); i++)
    {
        if (i->second->isInvisible(false))
            continue;

        if (hasSelection && !i->second->hasSelection())
            continue;

        float _distance = (i->second->center() - a->center()).length();
        if (_distance > distance)
        {
            distance = _distance;
            *b = i->second;
        }
    }
}


void ObjectSet::ritterSphere(QVector3D *center, float *radius, bool hasSelection)
{
    for (auto i = DisplayObject::begin(); i != DisplayObject::end(); i++)
    {
        if (i->second->isInvisible(false))
            continue;

        if (hasSelection && !i->second->hasSelection())
            continue;

        float d = (i->second->center() - (*center)).length();
        if (d > (*radius))
        {
            *center = ((d + (*radius))/2 * (*center) + (d - (*radius))/2 * i->second->center()) / d;
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
