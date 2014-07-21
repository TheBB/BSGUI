#include <set>

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QVector3D>
#include <QVector4D>

#ifndef _DISPOBJECT_H_
#define _DISPOBJECT_H_

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;

//! \brief Convenience struct used to build the OpenGL face index buffers.
//!
//! Allows us to work with a vector of quads instead of having a separate
//! index for each element of a quad.
typedef struct { GLuint a, b, c, d; } quad;

//! \brief Convenience struct used to build the OpenGL element index buffers.
//!
//! Allows us to work with a vector of pairs instead of having a separate
//! index for each element of a pair.
typedef struct { GLuint a, b; } pair;

//! \brief Represents a drawable 3D object.
//!
//! This class owns its own OpenGL buffer objects, and draws them when the `draw()` method is called.
//! It also maintains its own state (selected, etc.), and will draw itself accordingly.
class DispObject
{
public:
    //! \brief Create and prepare a display object.
    //!
    //! This computes all the vertex and face data and stores them in normal memory. It should perform
    //! as many expensive computations as possible, so that the `init()` call is correspondingly cheap.
    DispObject(QVector3D);

    virtual ~DispObject();

    //! \brief Check if the display object has been initialized.
    //!
    //! In this context, 'initialized' means having created the necessary OpenGL buffer objects.
    //! This has to happen after construction with the OpenGL context set. See `init()`.
    bool initialized() { return _initialized; };


    //! \brief Initializes the display object in the current OpenGL context.
    //!
    //! This method creates all the buffer objects necessary to draw this object. To make this as cheap
    //! as possible, the relevant information should have been pre-computed in the constructor so far
    //! as possible.
    //!
    //! It is assumed that the OpenGL context has been set before this method is called!
    void init();


    //! \brief Draws the object to the current OpenGL context.
    //!
    //! \param mvp Model-view-projection matrix.
    //! \param vprog OpenGL shader program used to render variable-colored data.
    //! \param cprog OpenGL shader program used to render constant-colored data.
    //!
    //! It is assumed that the OpenGL context has been set before this method is called!
    void draw(QMatrix4x4& mvp, QOpenGLShaderProgram& vprog, QOpenGLShaderProgram& cprog, bool picking);


    //! Returns the center of the approximate minimal bounding sphere of the object.
    QVector3D center() { return _center; };

    //! Returns the radius of the approximate minimal bounding sphere of the object.
    float radius() { return _radius; }


    void clearSelection();
    void selectFace(int idx);
    inline bool isFaceSelected(int idx) { return selectedFaces.find(idx) != selectedFaces.end(); }
    inline bool hasSelection() { return !selectedFaces.empty(); }
    inline bool fullSelection() { return selectedFaces.size() == 6; }


private:
    static uchar sColor[3];
    uchar color[3];
    static void incColors(uchar col[3], int num);

    bool _initialized;

    // Pre refinement
    ushort ntU, ntV, ntW;
    uint ntPtsU, ntPtsV, ntPtsW, ntPts;
    uint ntElems;

    // Refinement
    ushort rU, rV, rW;

    // Post refinement
    ushort nU, nV, nW;
    uint nPtsU, nPtsV, nPtsW, nPts, nElems, nElemLines, nLinesUV, nLinesUW, nLinesVW;

    // Bouding sphere
    QVector3D _center;
    float _radius;

    std::vector<QVector3D> vertexData;
    std::vector<QVector3D> vertexDataLines;
    std::vector<quad> faceData;
    std::vector<pair> boundaryData;
    std::vector<pair> elementData;

    QOpenGLBuffer vertexBuffer;
    QOpenGLBuffer vertexBufferLines;
    QOpenGLBuffer faceBuffer;
    QOpenGLBuffer boundaryBuffer;
    QOpenGLBuffer elementBuffer;

    std::set<uint> selectedFaces;
    std::set<uint> visibleFaces;
    std::set<uint> visibleBoundaries;
    std::set<uint> visibleElements;
    uint faceIdxs[7];
    uint boundaryIdxs[7];
    uint elementIdxs[7];

    void mkVertexData(QVector3D);
    void mkFaceData();
    void mkBoundaryData();
    void mkElementData();

    static void createBuffer(QOpenGLBuffer&);
    void mkBuffers();

    void boundingSphere();
    void farthestPointFrom(int index, int *found);
    void ritterSphere();

    void drawPicking(QMatrix4x4& mvp, QOpenGLShaderProgram& cprog);

    inline uint uvPt(uint i, uint j, bool posW)
    {
        return posW ?
            nPtsU*nPtsV + nPtsU*j + i :
            nPtsU*j + i;
    }

    inline uint uwPt(uint i, uint j, bool posV)
    {
        if (j == 0 || j == nW)
            return uvPt(i, posV ? nV : 0, j != 0);
        return posV ?
            2*nPtsU*nPtsV + nPtsU*(nPtsW-2) + nPtsU*(j-1) + i :
            2*nPtsU*nPtsV + nPtsU*(j-1) + i;
    }

    inline uint vwPt(uint i, uint j, bool posU)
    {
        if (j == 0 || j == nW)
            return uvPt(posU ? nU : 0, i, j != 0);
        if (i == 0 || i == nV)
            return uwPt(posU ? nU : 0, j, i != 0);
        return posU ?
            2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2) + (nPtsV-2)*(nPtsW-2) + (nPtsV-2)*(j-1) + (i-1) :
            2*nPtsU*nPtsV + 2*nPtsU*(nPtsW-2) + (nPtsV-2)*(j-1) + (i-1);
    }

    inline uint uvtPt(uint i, uint j, bool posW)
    {
        return posW ?
            ntPtsU*ntPtsV + ntPtsU*j + i :
            ntPtsU*j + i;
    }

    inline uint uwtPt(uint i, uint j, bool posV)
    {
        if (j == 0 || j == ntW)
            return uvtPt(i, posV ? ntV : 0, j != 0);
        return posV ?
            2*ntPtsU*ntPtsV + ntPtsU*(ntPtsW-2) + ntPtsU*(j-1) + i :
            2*ntPtsU*ntPtsV + ntPtsU*(j-1) + i;
    }

    inline uint vwtPt(uint i, uint j, bool posU)
    {
        if (j == 0 || j == ntW)
            return uvtPt(posU ? ntU : 0, i, j != 0);
        if (i == 0 || i == ntV)
            return uwtPt(posU ? ntU : 0, j, i != 0);
        return posU ?
            2*ntPtsU*ntPtsV + 2*ntPtsU*(ntPtsW-2) + (ntPtsV-2)*(ntPtsW-2) + (ntPtsV-2)*(j-1) + (i-1) :
            2*ntPtsU*ntPtsV + 2*ntPtsU*(ntPtsW-2) + (ntPtsV-2)*(j-1) + (i-1);
    }

    inline uint uvEl(uint i, uint j, bool posW)
    {
        return posW ?
            nU*nV + nU*j + i :
            nU*j + i;
    }

    inline uint uwEl(uint i, uint j, bool posV)
    {
        return posV ?
            2*nU*nV + nU*nW + nU*j + i :
            2*nU*nV + nU*j + i;
    }

    inline uint vwEl(uint i, uint j, bool posU)
    {
        return posU ?
            2*nU*nV + 2*nU*nW + nV*nW + nV*j + i :
            2*nU*nV + 2*nU*nW + nV*j + i;
    }

    inline uint uPbd(uint i, bool posV, bool posW, bool axV)
    {
        if (axV)
            return 4*(ntU+ntV) + (posV ? 2*(ntU+ntW) : 0) + (posW ? ntU : 0) + i;
        return (posV ? ntU : 0) + (posW ? 2*(ntU+ntV) : 0) + i;
    }

    inline uint vPbd(uint i, bool posU, bool posW, bool axU)
    {
        if (axU)
            return 4*(ntU+ntV) + 4*(ntU+ntW) + (posU ? 2*(ntV+ntW) : 0) + (posW ? ntV : 0) + i;
        return 2*ntU + (posU ? ntV : 0) + (posW ? 2*(ntU+ntV) : 0) + i;
    }

    inline uint wPbd(uint i, bool posU, bool posV, bool axU)
    {
        if (axU)
            return 4*(ntU+ntV) + 4*(ntU+ntW) + 2*ntV + (posU ? 2*(ntV+ntW) : 0) + (posV ? ntW : 0) + i;
        return 4*(ntU+ntV) + 2*ntU + (posU ? ntW : 0) + (posV ? 2*(ntU+ntW) : 0) + i;
    }

    inline uint uEll(uint i, int j, bool posO, bool axV)
    {
        if (axV)
            return 2*nLinesUV + (posO ? nLinesUW : 0) + ntU*j + i;
        return (posO ? nLinesUV : 0) + ntU*j + i;
    }

    inline uint vEll(uint i, int j, bool posO, bool axU)
    {
        if (axU)
            return 2*nLinesUV + 2*nLinesUW + (posO ? nLinesVW : 0) + ntV*j + i;
        return ntU*(ntV-1) + (posO ? nLinesUV : 0) + ntV*j + i;
    }

    inline uint wEll(uint i, int j, bool posO, bool axU)
    {
        if (axU)
            return 2*nLinesUV + 2*nLinesUW + ntV*(ntW-1) + (posO ? nLinesVW : 0) + ntW*j + i;
        return 2*nLinesUV + ntU*(ntW-1) + (posO ? nLinesUW : 0) + ntW*j + i;
    }
};

#endif /* _DISPOBJECT_H_ */
