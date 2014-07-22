#include <vector>
#include <set>
#include <unordered_map>

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>

#ifndef _DISPLAYOBJECT_H_
#define _DISPLAYOBJECT_H_

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef struct { GLuint a, b, c, d; } quad;
typedef struct { GLuint a, b; } pair;
enum ObjectType { OT_VOLUME, OT_FACE, OT_CURVE };
enum SelectionMode { SM_NONE, SM_PATCH, SM_FACE, SM_EDGE, SM_POINT };

class DisplayObject
{
public:
    DisplayObject(int parts);
    virtual ~DisplayObject();

    inline bool initialized() { return _initialized; }
    void initialize();

    inline bool visible() { return _visible; }

    void draw(QMatrix4x4 &mvp, QOpenGLShaderProgram &cprog);
    void drawPicking(QMatrix4x4 &mvp, QOpenGLShaderProgram &cprog);

    inline QVector3D center() { return _center; };
    inline float radius() { return _radius; }

    virtual uint nFaces() = 0;
    virtual uint nEdges() = 0;
    virtual uint nPoints() = 0;

    void selectionMode(SelectionMode mode, bool conjunction = true);
    void selectObject(bool selected);
    void selectFaces(bool selected, std::set<uint> faces);
    void selectEdges(bool selected, std::set<uint> edges);
    void selectPoints(bool selected, std::set<uint> points);

protected:
    QVector3D _center;
    float _radius;

    std::vector<QVector3D> vertexDataFaces;
    std::vector<QVector3D> vertexDataGrid;
    std::vector<quad> faceData;
    std::vector<pair> elementData, edgeData;
    std::vector<GLuint> pointData;

    bool _visible;
    std::set<uint> visibleFaces, visibleEdges, visiblePoints;
    std::vector<uint> faceIdxs, elementIdxs, edgeIdxs;

    std::unordered_map<uint, quad> faceEdgeMap;
    std::unordered_map<uint, pair> edgePointMap;

    void computeBoundingSphere();

private:
    bool _initialized;
    SelectionMode _selectionMode;
    uchar color[3];

    std::set<uint> selectedFaces, selectedEdges, selectedPoints;

    QOpenGLBuffer vertexBufferFaces, vertexBufferGrid, faceBuffer, elementBuffer, edgeBuffer, pointBuffer;

    void farthestPointFrom(QVector3D point, QVector3D *found);
    void ritterSphere();

    void refreshEdgesFromFaces();
    void refreshPointsFromEdges();
    void balloonEdgesToFaces(bool conjunction);
    void balloonPointsToEdges(bool conjunction);

    static void createBuffer(QOpenGLBuffer &buffer);

    static uchar sColor[3];
    static void incColors(uchar col[3], int num);
};

#endif /* _DISPLAYOBJECT_H_ */
