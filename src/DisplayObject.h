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

class DisplayObject
{
public:
    DisplayObject(int parts);
    virtual ~DisplayObject();

    inline bool initialized() { return _initialized; }
    void initialize();

    void draw(QMatrix4x4 &mvp, QOpenGLShaderProgram &cprog);
    void drawPicking(QMatrix4x4 &mvp, QOpenGLShaderProgram &cprog);

    inline QVector3D center() { return _center; };
    inline float radius() { return _radius; }

    virtual int nFaces() = 0;
    virtual int nEdges() = 0;
    virtual int nPoints() = 0;

protected:
    QVector3D _center;
    float _radius;

    std::vector<QVector3D> vertexDataFaces;
    std::vector<QVector3D> vertexDataGrid;
    std::vector<quad> faceData;
    std::vector<pair> elementData, edgeData;
    std::vector<GLuint> pointData;

    void computeBoundingSphere();

private:
    bool _initialized;
    uchar color[3];

    QOpenGLBuffer vertexBufferFaces, vertexBufferGrid, faceBuffer, elementBuffer, edgeBuffer, pointBuffer;

    void farthestPointFrom(QVector3D point, QVector3D *found);
    void ritterSphere();

    static void createBuffer(QOpenGLBuffer &buffer);

    static uchar sColor[3];
    static void incColors(uchar col[3], int num);
};

#endif /* _DISPLAYOBJECT_H_ */
