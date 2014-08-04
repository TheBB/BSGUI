#include <vector>
#include <set>
#include <map>
#include <mutex>
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
enum ObjectType { OT_VOLUME, OT_SURFACE, OT_CURVE };
enum SelectionMode { SM_PATCH, SM_FACE, SM_EDGE, SM_POINT };

class Patch;

//! \brief This is a superclass for all drawable objects (patches).
//!
//! A DisplayObject should always be owned by a Patch object (see ObjectSet for details).
//!
//! The DisplayObject class maintains a static index of all created objects, in
//! DisplayObject::indexMap. See \ref DisplayObjectIndex for more information.
//!
//! \link DisplayObject DisplayObjects \endlink come with an API to manipulate selection
//! and visibility of subcomponents (by which we mean faces, edges and vertices). See
//! \ref DisplayObjectComponents for more.
//!
//! To subclass DisplayObject, you must implement type(), nFaces(), nEdges() and
//! nPoints(). In addition, the constructor must initialize the #visibleFaces, #visibleEdges,
//! #visiblePoints, #faceIdxs, #elementIdxs, #edgeIdxs, #faceOffsets, #lineOffsets, #edgeOffsets,
//! #pointOffsets, #faceEdgeMap and #edgePointMap members. The superclass should then deal with
//! initialization and drawing for you.
class DisplayObject
{
public:
    //! \brief Constructs a DisplayObject and calls registerObject() to obtain an index.
    //! DisplayObject::m should be locked before calling.
    DisplayObject();

    //! \brief Frees the index held by this object by calling deregisterObject(), and then
    //! destroys the allocated OpenGL buffers if initialized.
    //! DisplayObject::m should be locked before calling.
    virtual ~DisplayObject();

    //! Returns the type of this object.
    virtual ObjectType type() = 0;

    //! Check whether the object has been initialized by the OpenGL engine.
    inline bool initialized() { return _initialized; }

    //! \brief Initialize this object. The caller must ensure that the OpenGL context is current.
    //!
    //! This will create the OpenGL buffers (#vertexBuffer, #normalBuffer, #faceBuffer,
    //! #elementBuffer, #edgeBuffer and #pointBuffer).
    void initialize();

    //! \brief Draws this object to the OpenGL buffer. The caller must ensure that the OpenGL
    //! context is current.
    //!
    //! \param mvp The model-view-projection matrix to transform augmented 4-vectors from
    //! model space to the OpenGL drawable surface.
    //! \param prog The OpenGL shader program to use.
    //! \param showPoints Whether to draw the vertices or not.
    void draw(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog, bool showPoints);

    //! \brief Draws this object to the OpenGL buffer for picking. The caller must ensure that
    //! the OpenGL context is current
    //!
    //! This draws all **visible** and **selectable** components (as determined by \a mode) in
    //! a single color each. The color used is determined by the global index of this
    //! DisplayObject. The caller can then use glReadPixels and DisplayObject::colorToIndex to
    //! find the objects in the selection area.
    //!
    //! \param mvp The model-view-projection matrix to transform augmented 4-vectors from
    //! model space to the OpenGL drawable surface.
    //! \param prog The OpenGL shader program to use.
    //! \param mode The current selection mode determines the appropriate coloring.
    void drawPicking(QMatrix4x4 &mvp, QOpenGLShaderProgram &prog, SelectionMode mode);

    //! Returns the center of the bounding sphere.
    inline QVector3D center() { return _center; };

    //! Returns the radius of the bounding sphere.
    inline float radius() { return _radius; }

    virtual uint nFaces() = 0; //!< Returns the number of faces in this object.
    virtual uint nEdges() = 0; //!< Returns the number of edges in this object.
    virtual uint nPoints() = 0; //!< Returns the number of points in this object.

    inline void setPatch(Patch *p) { _patch = p; } //!< Sets the Patch object that owns this object.
    inline Patch *patch() { return _patch; } //!< Returns the Patch object that owns this object.


    //! \defgroup DisplayObjectComponents DisplayObject component manipulation tools
    //! Each DisplayObject maintains the sets #selectedFaces, #selectedEdges and #selectedPoints.
    //! These are sets of integers denoting the indices of the selected components, and they
    //! determine whether a component is drawn blue (unselected) or orange (selected).
    //!
    //! The selection mode (ObjectSet::_selectionMode) has an impact on the functionality of
    //! many of these functions, but DisplayObject has no internal memory of the current selection
    //! mode.
    //!
    //! **NB:** that when a face is selected (say), the neighboring edges and vertices are also
    //! selected, even if the current selection mode is neither \c SM_EDGE or \c SM_POINT. This
    //! has an effect on drawing only!
    //!
    //! However, the function showSelected() will be faithful to the selection mode. In the above
    //! scenario, if an edge is selected and then hidden, the neighboring edges and vertices
    //! will remain visible.
    //!
    //! @{

    //! \brief Change selection mode.
    //!
    //! This will transform the currently selected objects according to the new mode.
    //!
    //! \param mode The new mode.
    //! \param conjunction If changing to a "higher" mode (e.g. from vertices to edges), this
    //! determines the new selection. If true, an edge will only be selected if both its vertices
    //! were already selected. If false, an edge will be selected if at least one of its vertices
    //! were already selected.
    void selectionMode(SelectionMode mode, bool conjunction = true);

    //! \brief Select or unselect the whole object.
    //!
    //! \param mode The current selection mode
    //! \param selected Whether to select or unselect the object.
    void selectObject(SelectionMode mode, bool selected);

    //! \brief Select or unselect faces. Assumes selection mode is `SM_FACE`.
    //!
    //! \param selected Whether to select or unselect.
    //! \param faces Set of face indices to change, between 0 and `nFaces() - 1` inclusive.
    void selectFaces(bool selected, std::set<uint> faces);

    //! \brief Select or unselect edges. Assumes selection mode is `SM_EDGE`.
    //!
    //! \param selected Whether to select or unselect.
    //! \param edges Set of edge indices to change, between 0 and `nEdges() - 1` inclusive.
    void selectEdges(bool selected, std::set<uint> edges);

    //! \brief Select or unselect vertices. Assumes selection mode is `SM_POINT`.
    //!
    //! \param selected Whether to select or unselect.
    //! \param points Set of vertices indices to change, between 0 and `nPoints() - 1` inclusive.
    void selectPoints(bool selected, std::set<uint> points);

    //! Check whether the object has any selected components.
    inline bool hasSelection()
    {
        return !selectedFaces.empty() || !selectedEdges.empty() || !selectedPoints.empty();
    }

    //! \brief Check whether the object is fully selected.
    //!
    //! \param mode The current selection mode
    bool fullSelection(SelectionMode mode);

    //! \brief Check whether the object is completely invisible.
    //!
    //! \param countPoints Whether a visible vertex counts as being visible.
    inline bool isInvisible(bool countPoints)
    {
        return visibleFaces.empty() && visibleEdges.empty() && (countPoints ? visiblePoints.empty() : true);
    }

    //! \brief Check whether the object is completely visible.
    //!
    //! \param countPoints Whether an invisible vertex counts as being invisibile.
    inline bool isFullyVisible(bool countPoints)
    {
        return (visibleFaces.size() == nFaces() &&
                visibleEdges.size() == nEdges() &&
                (countPoints ? visiblePoints.size() == nPoints() : true));
    }

    //! Check whether a given face is selected
    inline bool faceSelected(uint i) { return selectedFaces.find(i) != selectedFaces.end(); }

    //! Check whether a given edge is selected.
    inline bool edgeSelected(uint i) { return selectedEdges.find(i) != selectedEdges.end(); }

    //! Check whether a given vertex is selected.
    inline bool pointSelected(uint i) { return selectedPoints.find(i) != selectedPoints.end(); }

    //! \brief Shows or hides the currently selected components.
    //!
    //! \param mode The current selection mode.
    //! \param visible Whether to show or hide.
    void showSelected(SelectionMode mode, bool visible);

    //! @}


    //! \defgroup DisplayObjectIndex DisplayObject indexing system
    //! In order to do selection with the mouse, each DisplayObject is assigned a number of unique
    //! colors (#COLORS_PER_OBJECT). Currently this is 12, because this is the maximum needed, by
    //! Volume objects in edge selection mode. With 16 million colors (#NUM_COLORS), this gives
    //! about 1.4 million available color slots for objects to use (#NUM_INDICES).
    //!
    //! - By **index** we mean an integer between 0 and #NUM_INDICES-1.
    //! - By **key** we mean an integer between 0 and #NUM_COLORS-1. A key is simply a numerical
    //!   representation of a color. There are #COLORS_PER_OBJECT keys to each index.
    //!
    //! The mapping from indices to objects is stored in #indexMap.
    //!
    //! Please make sure you understand when to lock the mutex DisplayObject::m.
    //!
    //! @{

    //! \brief The mutex used for locking access to the object index (#indexMap).
    //!
    //! Lock this mutex when
    //!
    //! - Creating a new DisplayObject.
    //! - Destroying a DisplayObject
    //! - Interacting with the index.
    //!
    //! **It is always the caller's responsibility to lock this mutex. No methods belonging to
    //! DisplayObject, static or otherwise, will lock it.**
    static std::mutex m;

    //! \brief Returns the DisplayObject associated with the given index, or NULL if it doesn't exist.
    //! DisplayObject::m should be locked before calling.
    static DisplayObject *getObject(uint idx);

    //! \brief Convert color to key. By "key" we mean strictly a numerical representation of colors,
    //! i.e. (0,0,0) => 0, (0,0,1) => 1, (0,1,0) => 256, and so on.
    static uint colorToKey(GLubyte color[3]);

    //! \brief Convert key to index.
    //!
    //! \param key The key to convert.
    //! \retval index The index associated with this key (key / #COLORS_PER_OBJECT).
    //! \retval offset The offset (key % #COLORS_PER_OBJECT). 
    static void keyToIndex(uint key, uint *index, uint *offset);

    //! \brief Utility function to convert a color directly to an index.
    static void colorToIndex(GLubyte color[3], uint *index, uint *offset);

    //! For iterating over #indexMap.
    typedef typename std::map<uint, DisplayObject *>::iterator iterator;

    //! \brief An iterator pointing to the beginning of #indexMap.
    //! DisplayObject::m should be locked during iteration.
    static iterator begin() { return indexMap.begin(); }

    //! \brief An iterator pointing to the end of #indexMap.
    //! DisplayObject::m should be locked during iteration.
    static iterator end() { return indexMap.end(); }

    //! @}

protected:
    QVector3D _center; //!< Center of the bounding sphere.
    float _radius; //!< Radius of the bounding sphere.

    std::vector<QVector3D> vertexData, normalData;
    std::vector<quad> faceData;
    std::vector<pair> elementData, edgeData;
    std::vector<GLuint> pointData;

    std::set<uint> visibleFaces, visibleEdges, visiblePoints;
    std::vector<uint> faceIdxs, elementIdxs, edgeIdxs;
    std::vector<float> faceOffsets, lineOffsets, edgeOffsets, pointOffsets;

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

    //! DisplayObject::m should be locked before calling.
    static std::map<uint, DisplayObject *> indexMap;
    static uint nextIndex;
    //! DisplayObject::m should be locked before calling.
    static uint registerObject(DisplayObject *obj);
    //! DisplayObject::m should be locked before calling.
    static void deregisterObject(uint index);
    static QVector3D indexToColor(uint index, uint offset);
};

#endif /* _DISPLAYOBJECT_H_ */
