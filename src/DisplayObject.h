#include <vector>
#include <set>
#include <map>
#include <unordered_map>

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>

#ifndef _DISPLAYOBJECT_H_
#define _DISPLAYOBJECT_H_

#define NUM_COLORS 16777216
#define COLORS_PER_OBJECT 12
#define NUM_INDICES (NUM_COLORS/COLORS_PER_OBJECT)
#define WHITE_KEY (NUM_COLORS-1)

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef struct { GLuint a, b, c, d; } quad;
typedef struct { GLuint a, b; } pair;
enum ObjectType { OT_VOLUME };
enum SelectionMode { SM_PATCH, SM_FACE, SM_EDGE, SM_POINT };

class Patch;

class DisplayObject
{
public:
    DisplayObject();
    virtual ~DisplayObject();

    virtual ObjectType type() = 0;

    inline bool initialized() { return _initialized; }
    void initialize();

    void draw(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog, bool showPoints);
    void drawPicking(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog, SelectionMode mode);

    inline QVector3D center() { return _center; };
    inline float radius() { return _radius; }

    virtual uint nFaces() = 0;
    virtual uint nEdges() = 0;
    virtual uint nPoints() = 0;

    inline void setPatch(Patch *p) { _patch = p; }
    inline Patch *patch() { return _patch; }

    void selectionMode(SelectionMode mode, bool conjunction = true);
    void selectObject(SelectionMode mode, bool selected);
    void selectFaces(bool selected, std::set<uint> faces);
    void selectEdges(bool selected, std::set<uint> edges);
    void selectPoints(bool selected, std::set<uint> points);

    inline bool hasSelection()
    {
        return !selectedFaces.empty() || !selectedEdges.empty() || !selectedPoints.empty();
    }
    bool fullSelection(SelectionMode mode);

    inline bool isInvisible(bool countPoints)
    {
        return visibleFaces.empty() && visibleEdges.empty() && (countPoints ? visiblePoints.empty() : true);
    }
    inline bool isFullyVisible(bool countPoints)
    {
        return (visibleFaces.size() == nFaces() &&
                visibleEdges.size() == nEdges() &&
                (countPoints ? visiblePoints.size() == nPoints() : true));
    }

    inline bool faceSelected(uint i) { return selectedFaces.find(i) != selectedFaces.end(); }
    inline bool edgeSelected(uint i) { return selectedEdges.find(i) != selectedEdges.end(); }
    inline bool pointSelected(uint i) { return selectedPoints.find(i) != selectedPoints.end(); }

    void showSelected(SelectionMode mode, bool visible);

    static DisplayObject *getObject(uint idx);
    static uint colorToKey(GLubyte color[3]);
    static void keyToIndex(uint key, uint *index, uint *offset);
    static void colorToIndex(GLubyte color[3], uint *index, uint *offset);

protected:
    QVector3D _center;
    float _radius;

    std::vector<QVector3D> vertexData, normalData;
    std::vector<quad> faceData;
    std::vector<pair> elementData, edgeData;
    std::vector<GLuint> pointData;

    std::set<uint> visibleFaces, visibleEdges, visiblePoints;
    std::vector<uint> faceIdxs, elementIdxs, edgeIdxs;

    std::unordered_map<uint, quad> faceEdgeMap;
    std::unordered_map<uint, pair> edgePointMap;

    void computeBoundingSphere();
    void mkSamples(const std::vector<double> &knots, std::vector<double> &params, uint ref);

private:
    uint _index;
    bool _initialized;
    Patch *_patch;

    std::set<uint> selectedFaces, selectedEdges, selectedPoints;
    QOpenGLBuffer vertexBuffer, normalBuffer, faceBuffer, elementBuffer, edgeBuffer, pointBuffer;

    void farthestPointFrom(QVector3D point, QVector3D *found);
    void ritterSphere();

    void refreshEdgesFromFaces();
    void refreshPointsFromEdges();
    void balloonEdgesToFaces(bool conjunction);
    void balloonPointsToEdges(bool conjunction);

    static void createBuffer(QOpenGLBuffer &buffer);
    static void bindBuffer(QOpenGLShaderProgram &prog, QOpenGLBuffer &buffer, const char *attribute);
    static void setUniforms(QOpenGLShaderProgram&, QMatrix4x4, QVector3D, float);
    static void setUniforms(QOpenGLShaderProgram&, QMatrix4x4, uchar *, float);

    static std::map<uint, DisplayObject *> indexMap;
    static uint nextIndex;
    static uint registerObject(DisplayObject *obj);
    static void deregisterObject(uint index);
    static QVector3D indexToColor(uint index, uint offset);
};

#endif /* _DISPLAYOBJECT_H_ */
