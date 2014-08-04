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
//! nPoints(). In addition, the constructor must initialize a number of protected members.
//! These are described in \ref DisplayObjectSubclassing.
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
    //! mode. This is arguably a mistake, since things get complicated.
    //!
    //! **NB:** When a face is selected (say), the neighboring edges and vertices are also
    //! selected (in the sense that the #selectedEdges and #selectedPoints sets will not be empty),
    //! even if the current selection mode is neither \c SM_EDGE or \c SM_POINT. This
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


    //! \defgroup DisplayObjectSubclassing DisplayObject subclassing
    //! The constructor of the subclass should populate each of these members. They define the
    //! shape and topology of the DisplayObject, necessary for the \ref DisplayObjectComponents
    //! and the OpenGL drawing functions to work.
    //!
    //! @{

    //! The vertices of this object. The order is not important (for the superclass).
    std::vector<QVector3D> vertexData;

    //! The outward-facing normals (if applicable). The indexing must correspond to #vertexData.
    std::vector<QVector3D> normalData;

    //! A quad of indices for each quadrilateral polygon to draw. Indices must correspond to #vertexData.
    std::vector<quad> faceData;

    //! \brief A pair of indices for each element line to draw. These are the thin blue lines
    //! inside faces. Indices must correspond to #vertexData.
    std::vector<pair> elementData;

    //! \brief A pair of indices for each edge to draw. These are the thick black lines denoting
    //! the actual topological edges. Indices must correspond to #vertexData.
    std::vector<pair> edgeData;

    //! \brief An index for each vertex to draw. These are drawn as black circles.
    //! Indices must correspond to #vertexData.
    std::vector<GLuint> pointData;

    //! The indices of the visible faces.
    std::set<uint> visibleFaces;

    //! The indices of the visible edges.
    std::set<uint> visibleEdges;

    //! \brief The indices of the visible vertices. Note that vertex drawing can be overridden.
    //! The vertices should still be visible internally!
    std::set<uint> visiblePoints;

    //! \brief Index bounds for faces.
    //!
    //! Should be of size nFaces()+1. E.g. if the first 20 entries in #faceData make up the first
    //! face, and the second 40 make up the second face, then #faceIdxs should be {0,20,60}.
    //! Used by draw() to find the right indices when not all faces are visible.
    std::vector<uint> faceIdxs;

    //! \brief Index bounds for elements. Works like #faceIdxs, but for #elementData.
    std::vector<uint> elementIdxs;

    //! \brief Index bounds for edges. Works like #faceIdxs, but for #edgeData.
    std::vector<uint> edgeIdxs;


    //! \brief Normal offsets used for drawing faces.
    //!
    //! Each vertex **v** is drawn at **v** + *c* **n**, where **n** is the outward facing normal at
    //! that point. This can be useful for drawing lines slightly outside of faces to make them more
    //! visible. This vector contains the offsets used for faces. A value of zero will mean drawing
    //! at the actual location.
    std::vector<float> faceOffsets;

    //! \brief Normal offsets used for drawing element lines. See #faceOffsets.
    //!
    //! It can be useful to have two entries here, one for each side of the face.
    std::vector<float> lineOffsets;

    //! \brief Normal offsets used for drawing edges. See #faceOffsets.
    std::vector<float> edgeOffsets;

    //! \brief Normal offsets used for drawing edges. See #faceOffsets.
    std::vector<float> pointOffsets;

    //! A map from face index to edge indices describing the topology of this object.
    std::unordered_map<uint, quad> faceEdgeMap;

    //! A map from edge index to vertex indices describing the topology of this object.
    std::unordered_map<uint, pair> edgePointMap;

    //! @}


    //! \brief Computes (estimates) the minimal bounding sphere of this object using
    //! #vertexData, and saves the results to #_center and #_radius.
    void computeBoundingSphere();

    //! Utility function that refines a vector *knot* of knot values by inserting *ref*
    //! points in each element, storing the results in *params*.
    static void mkSamples(const std::vector<double> &knots, std::vector<double> &params, uint ref);

private:
    //! Index of this object. (See \ref DisplayObjectIndex for details.)
    uint _index;

    //! True if this object has been initialized.
    bool _initialized;

    //! The Patch object that owns this.
    Patch *_patch;


    //! \addtogroup DisplayObjectComponents
    //! @{
    std::set<uint> selectedFaces;
    std::set<uint> selectedEdges;
    std::set<uint> selectedPoints;
    //! @}


    //! OpenGL buffer corresponding to #vertexData.
    QOpenGLBuffer vertexBuffer;

    //! OpenGL buffer corresponding to #normalData.
    QOpenGLBuffer normalBuffer;

    //! OpenGL buffer corresponding to #faceData.
    QOpenGLBuffer faceBuffer;

    //! OpenGL buffer corresponding to #elementData.
    QOpenGLBuffer elementBuffer;

    //! OpenGL buffer corresponding to #edgeData.
    QOpenGLBuffer edgeBuffer;

    //! OpenGL buffer corresponding to #pointData.
    QOpenGLBuffer pointBuffer;


    //! \brief Computes, among the points in #vertexData, the one farthest from *point*.
    //!
    //! \retval found The most distant point.
    void farthestPointFrom(QVector3D point, QVector3D *found);

    //! \brief Computes (estimates) the minimal bounding sphere using the Ritter algorithm.
    //!
    //! Assumes that #_center and #_radius are decent guesses that might not cover the whole object.
    void ritterSphere();


    //! Selects only those edges that are neighboring a selected face.
    void refreshEdgesFromFaces();

    //! Selects only those points that are neighboring a selected edge.
    void refreshPointsFromEdges();

    //! \brief Balloons a selection of edges to a selection of faces.
    //!
    //! \param conjunction If *true* a face needs **all** its neighboring edges to become selected.
    //! If *false*, it needs only one.
    void balloonEdgesToFaces(bool conjunction);

    //! \brief Balloons a selection of vertices to a selection of edges.
    //! \param conjunction If *true* an edge needs *both* its neighboring vertices to become selected.
    //! If *false*, it needs only one.
    void balloonPointsToEdges(bool conjunction);

    //! Creates and binds an OpenGL buffer with static draw usage pattern.
    static void createBuffer(QOpenGLBuffer &buffer);

    //! \brief Binds an OpenGL buffer to the given program.
    //! \param prog Program to bind to.
    //! \param buffer OpenGL buffer object to bind.
    //! \param attribute Program attribute to bind to.
    static void bindBuffer(QOpenGLShaderProgram &prog, QOpenGLBuffer &buffer, const char *attribute);

    //! \brief Sets the uniform values in the shader program.
    //! \param prog Program to bind to.
    //! \param mvp Model-view-projection matrix to transform augmented 4-vectors from model space
    //! to the drawing surface.
    //! \param col Color to draw in .
    //! \param p Normal offset (see #faceOffsets).
    static void setUniforms(QOpenGLShaderProgram& prog, QMatrix4x4 mvp, QVector3D col, float p);
    static void setUniforms(QOpenGLShaderProgram& prog, QMatrix4x4 mvp, uchar *col, float p);

    //! \addtogroup DisplayObjectIndex
    //! @{

    //! \brief The map from indices to \link DisplayObject DisplayObjects \endlink.
    //! DisplayObject::m should be locked before manipulating.
    static std::map<uint, DisplayObject *> indexMap;

    //! The next available index, used by #registerObject as a cache.
    static uint nextIndex;

    //! \brief Register a new DisplayObject and get an index.
    //! DisplayObject::m should be locked before calling.
    static uint registerObject(DisplayObject *obj);

    //! \brief Unregister a DisplayObject and free the index.
    //! DisplayObject::m should be locked before calling.
    static void deregisterObject(uint index);

    //! \brief Convert an index to a color.
    static QVector3D indexToColor(uint index, uint offset);

    //! @}
};

#endif /* _DISPLAYOBJECT_H_ */
