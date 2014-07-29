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
enum ObjectType { OT_VOLUME };
enum SelectionMode { SM_PATCH, SM_FACE, SM_EDGE, SM_POINT };

class DisplayObject
{
public:
    DisplayObject(int parts);
    virtual ~DisplayObject();

    virtual ObjectType type() = 0;

    inline bool initialized() { return _initialized; }
    void initialize();

    inline bool visible() { return _visible; }

    void draw(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog, bool showPoints);
    void drawPicking(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog, SelectionMode mode);

    inline QVector3D center() { return _center; };
    inline float radius() { return _radius; }

    virtual uint nFaces() = 0;
    virtual uint nEdges() = 0;
    virtual uint nPoints() = 0;

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

    inline bool faceSelected(uint i) { return selectedFaces.find(i) != selectedFaces.end(); }
    inline bool edgeSelected(uint i) { return selectedEdges.find(i) != selectedEdges.end(); }
    inline bool pointSelected(uint i) { return selectedPoints.find(i) != selectedPoints.end(); }

    void showSelected(SelectionMode mode, bool visible);

    inline bool hasColor(uint color) { return minColor <= color && color < maxColor; }
    inline uint baseColor() { return minColor; }

protected:
    QVector3D _center;
    float _radius;

    std::vector<QVector3D> vertexData, normalData;
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
    uchar color[3];
    uint minColor, maxColor;

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
    
    static uchar sColor[3];
    static void incColors(uchar col[3], int num);
};

#endif /* _DISPLAYOBJECT_H_ */
