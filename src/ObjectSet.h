#include <fstream>
#include <mutex>
#include <set>
#include <string>
#include <vector>
#include <QAbstractItemModel>
#include <QFileInfo>
#include <QItemSelection>
#include <QModelIndex>
#include <QString>
#include <QVector3D>

#include "DisplayObject.h"

#ifndef _OBJECTSET_H_
#define _OBJECTSET_H_

enum NodeType { NT_ROOT, NT_FILE, NT_PATCH, NT_COMPONENTS, NT_COMPONENT };
enum ComponentType { CT_FACE, CT_EDGE, CT_POINT };

class Node
{
public:
    Node(Node *parent = NULL);
    ~Node();

    virtual NodeType type() { return NT_ROOT; }
    virtual QString displayString() { return "###"; }

    void addChild(Node *child);
    Node *getChild(int idx);
    int indexOfChild(Node *child);
    int indexInParent();
    int nChildren();

    inline Node *parent() { return _parent; }

    inline std::vector<Node *> children() { return _children; }

protected:
    Node *_parent;
    std::vector<Node *> _children;
};


class File : public Node
{
public:
    File(QString fn, Node *parent = NULL);
    ~File() { }

    NodeType type() { return NT_FILE; }
    QString displayString();

    inline bool matches(QString fn)
    {
        return fn == "" && absolutePath == "" || QFileInfo(fn).absoluteFilePath() == absolutePath;
    }

    inline QString fn() { return fileName; }

private:
    QString fileName, absolutePath;
};


class Patch : public Node
{
public:
    Patch(DisplayObject *obj, Node *parent = NULL);
    ~Patch();

    virtual NodeType type() { return NT_PATCH; }
    virtual QString displayString();

    inline DisplayObject *obj() { return _obj; }

private:
    DisplayObject *_obj;
};


class Components : public Node
{
public:
    Components(ComponentType type, Node *parent = NULL);
    ~Components() {};

    virtual NodeType type () { return NT_COMPONENTS; }
    virtual QString displayString();

    inline ComponentType cType() { return _type; }

private:
    ComponentType _type;
};


class Component : public Node
{
public:

    Component(uint index, Node *parent = NULL);
    ~Component() {};

    virtual NodeType type () { return NT_COMPONENT; }
    virtual QString displayString();

    bool isSelected();

    inline uint index() { return _index; }

private:
    uint _index;
};


class ObjectSet : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ObjectSet(QObject *parent = NULL);
    ~ObjectSet();

    bool hasSelection() { return !selectedObjects.empty(); }
    inline SelectionMode selectionMode() { return _selectionMode; }
    void setSelectionMode(SelectionMode mode, bool fromMouse);
    void showSelected(bool visible);
    void showAllSelectedPatches(bool visible);
    void showAll();

    std::mutex m;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void addPatchesFromFile(std::string fileName);
    void boundingSphere(QVector3D *center, float *radius);
    void setSelection(std::set<uint> *picks, bool clear = true);
    void addToSelection(Node *node, bool signal = true);
    void removeFromSelection(Node *node, bool signal = true);

    typedef typename std::vector<Patch *>::iterator iterator;
    typedef typename std::vector<Patch *>::const_iterator const_iterator;
    iterator begin() { return displayObjects.begin(); }
    const_iterator begin() const { return displayObjects.begin(); }
    const_iterator cbegin() const { return displayObjects.cbegin(); }
    iterator end() { return displayObjects.end(); }
    const_iterator end() const { return displayObjects.end(); }
    const_iterator cend() const { return displayObjects.cend(); }

signals:
    void requestInitialization(DisplayObject *obj);
    void update();
    void selectionChanged();
    void selectionModeChanged(SelectionMode mode, bool fromMouse);

private:
    Node *root;
    Node *getOrCreateFileNode(QString fileName);

    SelectionMode _selectionMode;

    std::vector<Patch *> displayObjects;
    std::set<Patch *> selectedObjects;

    void farthestPointFrom(DisplayObject *a, DisplayObject **b, std::vector<Patch *> *vec);
    void ritterSphere(QVector3D *center, float *radius, std::vector<Patch *> *vec);

    void signalCheckChange(Patch *patch);
    void signalVisibleChange(Patch *patch);
};

#endif /* _OBJECTSET_H_ */
