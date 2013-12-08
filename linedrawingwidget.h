/*
#-------------------------------------------------
#
# LineDrawingWidget created by QtCreator
#
# @Author: zdd
# @Email: zddhub@gmail.com
#
# rstc.cc in Qt
#
# Thanks:
#    Original by Tilke Judd
#    Tweaks by Szymon Rusinkiewicz
#
#    apparentridge.h
#    Compute apparent ridges.
#
#    Implements method of
#      Judd, T., Durand, F, and Adelson, E.
#      Apparent Ridges for Line Drawing,
#      ACM Trans. Graphics (Proc. SIGGRAPH), vol. 26, no. 3, 2007.
#-------------------------------------------------
*/


#ifndef LINEDRAWINGWIDGET_H
#define LINEDRAWINGWIDGET_H

#include <QGLWidget>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>

#include <stdio.h>
#include <stdlib.h>
#include "TriMesh.h"
#include "TriMesh_algo.h"
#include "XForm.h"
#include "GLCamera.h"
#include "timestamp.h"
#include <algorithm>

using namespace std;




class LineDrawingWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit LineDrawingWidget(QWidget *parent = 0);
    bool readMesh(const char *filename, const char* xffilename = "");
    void clearMesh();
signals:

public slots:

protected:
    void resizeGL(int width, int height);
    void paintGL();

    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);

    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *);

private:
    void reset();
    void init_rtsc();
private:
    bool isCtrlPressed;
    char xfFileName[1024];

private:
    // Draw triangle strips.  They are stored as length followed by values.
    void draw_tstrips();
    // Create a texture with a black line of the given width.
    void make_texture(float width);
    // Draw contours and suggestive contours using texture mapping
    void draw_c_sc_texture(const vector<float> &ndotv,
                           const vector<float> &kr,
                           const vector<float> &sctest_num,
                           const vector<float> &sctest_den);
    // Color the mesh by curvatures
    void compute_curv_colors();
    // Similar, but grayscale mapping of mean curvature H
    void compute_gcurv_colors();
    // Set up textures to be used for the lighting.
    // These are indexed by (n dot l), though they are actually 2D textures
    // with a height of 1 because some hardware (cough, cough, ATI) is
    // thoroughly broken for 1D textures...
    void make_light_textures(GLuint *texture_contexts);
    // Draw the basic mesh, which we'll overlay with lines
    void draw_base_mesh();
    // Compute per-vertex n dot l, n dot v, radial curvature, and
    // derivative of curvature for the current view
    void compute_perview(vector<float> &ndotv, vector<float> &kr,
                         vector<float> &sctest_num, vector<float> &sctest_den,
                         vector<float> &shtest_num, vector<float> &q1,
                         vector<vec2> &t1, vector<float> &Dt1q1,
                         bool extra_sin2theta = false);
    // Compute gradient of (kr * sin^2 theta) at vertex i
    inline vec gradkr(int i);
    // Find a zero crossing between val0 and val1 by linear interpolation
    // Returns 0 if zero crossing is at val0, 1 if at val1, etc.
    static inline float find_zero_linear(float val0, float val1);
    // Find a zero crossing using Hermite interpolation
    float find_zero_hermite(int v0, int v1, float val0, float val1,
                            const vec &grad0, const vec &grad1);
    // Draw part of a zero-crossing curve on one triangle face, but only if
    // "test_num/test_den" is positive.  v0,v1,v2 are the indices of the 3
    // vertices, "val" are the values of the scalar field whose zero
    // crossings we are finding, and "test_*" are the values we are testing
    // to make sure they are positive.  This function assumes that val0 has
    // opposite sign from val1 and val2 - the following function is the
    // general one that figures out which one actually has the different sign.
    void draw_face_isoline2(int v0, int v1, int v2,
                            const vector<float> &val,
                            const vector<float> &test_num,
                            const vector<float> &test_den,
                            bool do_hermite, bool do_test, float fade);
    // See above.  This is the driver function that figures out which of
    // v0, v1, v2 has a different sign from the others.
    void draw_face_isoline(int v0, int v1, int v2,
                           const vector<float> &val,
                           const vector<float> &test_num,
                           const vector<float> &test_den,
                           const vector<float> &ndotv,
                           bool do_bfcull, bool do_hermite,
                           bool do_test, float fade);
    // Takes a scalar field and renders the zero crossings, but only where
    // test_num/test_den is greater than 0.
    void draw_isolines(const vector<float> &val,
                       const vector<float> &test_num,
                       const vector<float> &test_den,
                       const vector<float> &ndotv,
                       bool do_bfcull, bool do_hermite,
                       bool do_test, float fade);
    // Draw part of a ridge/valley curve on one triangle face.  v0,v1,v2
    // are the indices of the 3 vertices; this function assumes that the
    // curve connects points on the edges v0-v1 and v1-v2
    // (or connects point on v0-v1 to center if to_center is true)
    void draw_segment_ridge(int v0, int v1, int v2,
                            float emax0, float emax1, float emax2,
                            float kmax0, float kmax1, float kmax2,
                            float thresh, bool to_center);
    // Draw ridges or valleys (depending on do_ridge) in a triangle v0,v1,v2
    // - uses ndotv for backface culling (enabled with do_bfcull)
    // - do_test checks for curvature maxima/minina for ridges/valleys
    //   (when off, it draws positive minima and negative maxima)
    // Note: this computes ridges/valleys every time, instead of once at the
    //   start (given they aren't view dependent, this is wasteful)
    // Algorithm based on formulas of Ohtake et al., 2004.
    void draw_face_ridges(int v0, int v1, int v2,
                          bool do_ridge,
                          const vector<float> &ndotv,
                          bool do_bfcull, bool do_test, float thresh);
    // Draw the ridges (valleys) of the mesh
    void draw_mesh_ridges(bool do_ridge, const vector<float> &ndotv,
                          bool do_bfcull, bool do_test, float thresh);
    // Draw principal highlights on a face
    void draw_face_ph(int v0, int v1, int v2, bool do_ridge,
                      const vector<float> &ndotv, bool do_bfcull,
                      bool do_test, float thresh);
    // Draw principal highlights
    void draw_mesh_ph(bool do_ridge, const vector<float> &ndotv, bool do_bfcull,
                      bool do_test, float thresh);
    // Draw exterior silhouette of the mesh: this just draws
    // thick contours, which are partially hidden by the mesh.
    // Note: this needs to happen *before* draw_base_mesh...
    void draw_silhouette(const vector<float> &ndotv);
    // Draw the boundaries on the mesh
    void draw_boundaries(bool do_hidden);
    // Draw lines of n.l = const.
    void draw_isophotes(const vector<float> &ndotv);
    // Draw lines of constant depth
    void draw_topolines(const vector<float> &ndotv);
    // Draw K=0, H=0, and DwKr=thresh lines
    void draw_misc(const vector<float> &ndotv, const vector<float> &DwKr,
                   bool do_hidden);
    // Draw the mesh, possibly including a bunch of lines
    void draw_mesh();
    // Clear the screen and reset OpenGL modes to something sane
    void cls();
    // Set up viewport and scissoring for the subwindow, and optionally draw
    // a box around it (actually, just clears a rectangle one pixel bigger
    // to black).  Assumes current viewport is set up for the whole window.
    void set_subwindow_viewport(bool draw_box = false);
    // Set the view to look at the middle of the mesh, from reasonably far away
    void resetview();

    // Smooth the mesh
    void filter_mesh(int dummy = 0);
    // Diffuse the normals across the mesh
    void filter_normals(int dummy = 0);
    // Diffuse the curvatures across the mesh
    void filter_curv(int dummy = 0);
    // Diffuse the curvature derivatives across the mesh
    void filter_dcurv(int dummy = 0);
    // Perform an iteration of subdivision
    void subdivide_mesh(int dummy = 0);

    // Compute a "feature size" for the mesh: computed as 1% of
    // the reciprocal of the 10-th percentile curvature
    void compute_feature_size();
private:
    //rtsc
    //  mesh...
    TriMesh *themesh;

    // Two cameras: the primary one, and an alternate one to fix the lines
    // and see them from a different direction
    int dual_vpmode, mouse_moves_alt;
    GLCamera camera, camera_alt;
    xform xf, xf_alt;
    float fov;
    double alt_projmatrix[16];
    char *xffilename; // Filename where we look for "home" position
    point viewpos;    // Current view position

    // Toggles for drawing various lines
    int draw_extsil, draw_c, draw_sc;
    int draw_sh, draw_phridges, draw_phvalleys ;
    int draw_ridges, draw_valleys, draw_apparent;
    int draw_K, draw_H, draw_DwKr;
    int draw_bdy, draw_isoph, draw_topo;
    int niso, ntopo;
    float topo_offset;

    // Toggles for tests we perform
    int draw_hidden;
    int test_c, test_sc, test_sh, test_ph, test_rv, test_ar;
    float sug_thresh, sh_thresh, ph_thresh;
    float rv_thresh, ar_thresh;

    // Toggles for style
    int use_texture;
    int draw_faded;
    int draw_colors;
    int use_hermite;

    // Mesh colorization
    enum { COLOR_WHITE, COLOR_GRAY, COLOR_CURV, COLOR_GCURV, COLOR_MESH };
    //static const int ncolor_styles;
    int color_style;
    vector<Color> curv_colors, gcurv_colors;
    int draw_edges;

    // Lighting
    enum { LIGHTING_NONE, LIGHTING_LAMBERTIAN, LIGHTING_LAMBERTIAN2,
           LIGHTING_HEMISPHERE, LIGHTING_TOON, LIGHTING_TOONBW, LIGHTING_GOOCH };
    //static const int nlighting_styles;
    int lighting_style;
    //GLUI_Rotation *lightdir_glui = NULL;
    //float lightdir_matrix[16] = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    int light_wrt_camera;

    // Per-vertex vectors
    int draw_norm, draw_curv1, draw_curv2, draw_asymp;
    int draw_w, draw_wperp;

    // Other miscellaneous variables
    float feature_size;	// Used to make thresholds dimensionless
    float currsmooth;	// Used in smoothing
    vec currcolor;		// Current line color

private:
    //apparentridge.cc
    // Compute principal view-dependent curvatures and directions at vertex i.
    // ndotv = cosine of angle between normal and view direction
    // (u,v) = coordinates of w (projected view) in principal coordinates
    // Pass in u^2, u*v, and v^2, since those are readily available.
    // Fills in q1 and t1 (using the paper's notation).
    // Note that the latter is expressed in the (pdir1,pdir2) coordinate basis
    void compute_viewdep_curv(const TriMesh *mesh, int i, float ndotv,
                              float u2, float uv, float v2,
                              float &q1, vec2 &t1);
    // Compute D_{t_1} q_1 - the derivative of max view-dependent curvature
    // in the principal max view-dependent curvature direction.
    void compute_Dt1q1(const TriMesh *mesh, int i, float ndotv,
                       const vector<float> &q1, const vector<vec2> &t1,
                       float &Dt1q1);
    // Draw part of an apparent ridge/valley curve on one triangle face.
    // v0,v1,v2 are the indices of the 3 vertices; this function assumes that the
    // curve connects points on the edges v0-v1 and v1-v2
    // (or connects point on v0-v1 to center if to_center is true)
    void draw_segment_app_ridge(int v0, int v1, int v2,
                                float emax0, float emax1, float emax2,
                                float kmax0, float kmax1, float kmax2,
                                const vec &tmax0, const vec &tmax1, const vec &tmax2,
                                float thresh, bool to_center, bool do_test);

    // Draw apparent ridges in a triangle
    void draw_face_app_ridges(int v0, int v1, int v2,
                              const vector<float> &ndotv, const vector<float> &q1,
                              const vector<vec2> &t1, const vector<float> &Dt1q1,
                              bool do_bfcull, bool do_test, float thresh);
    // Draw apparent ridges of the mesh
    void draw_mesh_app_ridges(const vector<float> &ndotv, const vector<float> &q1,
                              const vector<vec2> &t1, const vector<float> &Dt1q1,
                              bool do_bfcull, bool do_test, float thresh);

};

#endif // LINEDRAWINGWIDGET_H
