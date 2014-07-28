#include <mutex>
#include <set>
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

enum NodeType { NT_ROOT, NT_FILE, NT_PATCH, /* NT_FACE */ };

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
    int numberOfChildren();

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


// class Face : public Node
// {
// public:
//     Face(int index, Node *parent = NULL);
//     ~Face() {};

//     NodeType type() { return NT_FACE; }
//     QString displayString();

// private:
//     int _index;
// };


class ObjectSet : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit ObjectSet(QObject *parent = NULL);
    ~ObjectSet();

    // bool hasSelection() { return !selectedObjects.empty(); }
    // bool selectFaces() { return _selectFaces; }
    // void setSelectFaces(bool val, bool fromMouse);
    inline SelectionMode selectionMode() { return _selectionMode; }

    std::mutex m;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void addCubeFromCenter(QVector3D center);
    void boundingSphere(QVector3D *center, float *radius);
    void setSelection(std::set<uint> *picks, bool clear = true);
    // void addToSelection(Node *node, bool signal = true);
    // void removeFromSelection(Node *node, bool signal = true);

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
    void selectFacesChanged(bool val, bool fromMouse);

private:
    Node *root;
    Node *getOrCreateFileNode(QString fileName);

    SelectionMode _selectionMode;

    std::vector<Patch *> displayObjects;
    std::set<Patch *> selectedObjects;
    // bool _selectFaces;

    void farthestPointFrom(DisplayObject *a, DisplayObject **b, std::vector<Patch *> *vec);
    void ritterSphere(QVector3D *center, float *radius, std::vector<Patch *> *vec);

    // void signalCheckChange(Patch *patch);
};

#endif /* _OBJECTSET_H_ */
