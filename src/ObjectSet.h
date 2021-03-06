/*
 * Copyright (C) 2014 SINTEF ICT,
 * Applied Mathematics, Norway.
 *
 * Contact information:
 * E-mail: eivind.fonn@sintef.no
 * SINTEF ICT, Department of Applied Mathematics,
 * P.O. Box 4760 Sluppen,
 * 7045 Trondheim, Norway.
 *
 * This file is part of BSGUI.
 *
 * BSGUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * BSGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with GoTools. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In accordance with Section 7(b) of the GNU Affero General Public
 * License, a covered work must retain the producer line in every data
 * file that is created or manipulated using GoTools.
 *
 * Other Usage
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial activities involving the GoTools library without
 * disclosing the source code of your own applications.
 *
 * This file may be used in accordance with the terms contained in a
 * written agreement between you and SINTEF ICT.
 */

#include <fstream>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>
#include <QAbstractItemModel>
#include <QDateTime>
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
enum LogLevel { LL_NORMAL, LL_WARNING, LL_ERROR, LL_FATAL };
enum FileChange { FC_NONE, FC_DELETED, FC_CHANGED, FC_CHANGING };

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

    void refreshInfo();

    inline bool matches(QString fn)
    {
        return fn == "" && absolutePath == "" || QFileInfo(fn).absoluteFilePath() == absolutePath;
    }

    inline QString fn() { return fileName; }
    inline QString absolute() { return absolutePath; }
    inline uint size() { return _size; }
    inline uint nChecksums() { return checksums.size(); }

    void checkChange();
    inline FileChange change() { return _change; }

    void clearPatches();

    std::mutex m;

private:
    QString fileName, absolutePath;
    std::vector<size_t> checksums;
    uint _size, lastCheckedSize;
    QDateTime modified;

    FileChange _change;

    void computeChecksums(std::vector<size_t> *vec);
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

    bool hasSelection();
    inline SelectionMode selectionMode() { return _selectionMode; }
    void setSelectionMode(SelectionMode mode);
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

    void loadFile(QString fileName);
    void boundingSphere(QVector3D *center, float *radius);
    void setSelection(std::set<std::pair<uint,uint>> *picks, bool clear = true);
    void addToSelection(Node *node, bool signal = true, bool lock = true);
    void removeFromSelection(Node *node, bool signal = true, bool lock = true);

signals:
    void requestInitialization(DisplayObject *obj);
    void update();
    void selectionChanged();
    void selectionModeChanged(SelectionMode mode);
    void log(QString, LogLevel = LL_NORMAL);

private:
    Node *root;
    File *getOrCreateFileNode(QString fileName);

    SelectionMode _selectionMode;
    
    std::thread fileWatcher;
    bool watch;
    void watchFiles();
    std::mutex mQueue;
    std::set<QString> loadQueue;

    void farthestPointFrom(DisplayObject *a, DisplayObject **b, bool hasSelection);
    void ritterSphere(QVector3D *center, float *radius, bool hasSelection);

    void signalCheckChange(Patch *patch);
    void signalVisibleChange(Patch *patch);

    void addPatchesFromFile(QString fileName);
    bool addPatchFromStream(std::ifstream &stream, File *file);
};

#endif /* _OBJECTSET_H_ */
