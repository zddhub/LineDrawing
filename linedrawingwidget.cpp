/*
#-------------------------------------------------
#
# LineDrawingWidget created by QtCreator 2013-01-24T10:25:01
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

#include "linedrawingwidget.h"
extern const int ncolor_styles = 5;
extern const int nlighting_styles = 7;
Mouse::button btn = Mouse::NONE;

LineDrawingWidget::LineDrawingWidget(QWidget *parent) :
    QGLWidget(parent)
{

    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setSizePolicy(sizePolicy);

    this->setFocusPolicy(Qt::StrongFocus);//QWidget只有设置焦点才能进行按键响应

    resize(840, 720);

    isCtrlPressed = false;

    themesh = NULL;
    init_rtsc();
}

void LineDrawingWidget::init_rtsc()
{
    // Two cameras: the primary one, and an alternate one to fix the lines
    // and see them from a different direction
    dual_vpmode = false, mouse_moves_alt = false;
    fov = 0.7f;

    // Toggles for drawing various lines
    draw_extsil = 0, draw_c = 1, draw_sc = 0;
    draw_sh = 0, draw_phridges = 0, draw_phvalleys = 0;
    draw_ridges = 0, draw_valleys = 0, draw_apparent = 0;
    draw_K = 0, draw_H = 0, draw_DwKr = 0;
    draw_bdy = 0, draw_isoph = 0, draw_topo = 0;
    niso = 20, ntopo = 20;
    topo_offset = 0.0f;

    // Toggles for tests we perform
    draw_hidden = 0;
    test_c = 1, test_sc = 1, test_sh = 1, test_ph = 1, test_rv = 1, test_ar = 1;
    sug_thresh = 0.01, sh_thresh = 0.02, ph_thresh = 0.04;
    rv_thresh = 0.1, ar_thresh = 0.1;

    // Toggles for style
    use_texture = 0;
    draw_faded = 1;
    draw_colors = 0;
    use_hermite = 0;

    // Mesh colorization
    color_style = COLOR_WHITE;
    draw_edges = false;

    // Lighting
    lighting_style = LIGHTING_NONE;
    light_wrt_camera = true;

    // Per-vertex vectors
    draw_norm = 0, draw_curv1 = 0, draw_curv2 = 0, draw_asymp = 0;
    draw_w = 0, draw_wperp = 0;
}

bool LineDrawingWidget::readMesh(const char *filename, const char* xffilename)
{
    if(themesh)
    {
        delete themesh;
        themesh = NULL;
    }

    themesh = TriMesh::read(filename);
    if(!themesh)
    {
        cout<<"read file "<<filename<<" error."<<endl;
        exit(-1);
    }
//    pca_rotate(themesh);

    themesh->need_tstrips();
    themesh->need_bsphere();
    themesh->need_normals();
    themesh->need_curvatures();
    themesh->need_dcurv();
    compute_feature_size();
    currsmooth = 0.5f * themesh->feature_size();

    //设置xf,使模型在视口之中
    strcpy(xfFileName, xffilename);

    if(!xf.read(xffilename))
        xf = xform::trans(0, 0, -3.5f / fov * themesh->bsphere.r) *
                             xform::trans(-themesh->bsphere.center);
    updateGL();
    return true;
}

void LineDrawingWidget::clearMesh()
{
    if(themesh)
    {
        delete themesh;
        themesh = NULL;
    }
    updateGL();
}

//------------------------protected function------------------------

void LineDrawingWidget::resizeGL(int width, int height)
{
    //设置opengl视口与QWidget窗口大小相同
    glViewport( 0, 0, (GLint)width, (GLint)height );
}

void LineDrawingWidget::paintGL()
{
    if(!themesh)
    {
        cls();
        return;
    }

    viewpos = inv(xf) * point(0,0,0);

    camera.setupGL(xf * themesh->bsphere.center, themesh->bsphere.r);

    cls();

    // Transform and draw
    glPushMatrix();
    glMultMatrixd((double *)xf);
    draw_mesh();
    glPopMatrix();
}

//鼠标交互处理
void LineDrawingWidget::mousePressEvent(QMouseEvent *e)
{
    if(!themesh)
        return;

    int x = e->pos().x();
    int y = e->pos().y();

    //根据鼠标交互位置(x,y)重新放置摄像机
    camera.mouse(x, y, btn, xf*themesh->bsphere.center, themesh->bsphere.r, xf);
    camera.setupGL(xf*themesh->bsphere.center, themesh->bsphere.r);
    //
    if(e->button() ==  Qt::LeftButton)
    {
        if(isCtrlPressed)
            btn = Mouse::LIGHT;
        else
            btn = Mouse::ROTATE;
    }
    else if(e->button() == Qt::RightButton)
    {
        if(isCtrlPressed)
            btn = Mouse::LIGHT;
        else
            btn = Mouse::MOVEZ;
    }
    else if(e->button() == Qt::MidButton)
    {
        btn = Mouse::MOVEXY;
    }
    else
        btn = Mouse::NONE;

    mouseMoveEvent(e);
}

void LineDrawingWidget::mouseReleaseEvent(QMouseEvent * /*e*/)
{
    btn = Mouse::NONE;
}

void LineDrawingWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(!themesh)
        return;

    int x = e->pos().x();
    int y = e->pos().y();


    camera.mouse(x, y, btn, xf*themesh->bsphere.center, themesh->bsphere.r, xf);

    if(btn != Mouse::NONE)
        updateGL();
}

void LineDrawingWidget::wheelEvent(QWheelEvent *e)
{
    if(!themesh)
        return;

    int x = e->pos().x();
    int y = e->pos().y();

    if(e->orientation() == Qt::Vertical)
    {
        if (e->delta() > 0)
        {
            btn = Mouse::WHEELUP;
        }
        else
        {
            btn = Mouse::WHEELDOWN;
        }
    }

    e->accept();
    camera.mouse(x, y, btn, xf*themesh->bsphere.center, themesh->bsphere.r, xf);
    btn = Mouse::NONE;
    updateGL();
}

void LineDrawingWidget::keyPressEvent(QKeyEvent *e)
{
    if(!themesh)
        return;

    switch(e->key())
    {
    case Qt::Key_Control:
        isCtrlPressed = true;
        break;
    case Qt::Key_Space:
        reset();
        break;
    case Qt::Key_A:
        draw_apparent = !draw_apparent;
        break;
    case Qt::Key_B:
        draw_bdy = !draw_bdy;
        break;
    case Qt::Key_Y:
        draw_colors = !draw_colors;
        break;
    case Qt::Key_W:
        draw_c = !draw_c;
        break;
    case Qt::Key_S:
        draw_sc = !draw_sc;
        break;
    case Qt::Key_E:
        draw_edges = !draw_edges;
        break;
    case Qt::Key_T:
        draw_extsil = !draw_extsil;
        break;
    case Qt::Key_I:
        draw_isoph = !draw_isoph;
        break;
    case Qt::Key_L:
        lighting_style++;
        lighting_style %= nlighting_styles;
        break;
    case Qt::Key_O:
        color_style++;
        color_style %= ncolor_styles-1;
        break;
    case Qt::Key_N:
        draw_norm = !draw_norm;
        break;
    case Qt::Key_0:
        rv_thresh /= 1.1f;
        break;
    case Qt::Key_1:
        rv_thresh *= 1.1f;
        break;
    case Qt::Key_3:
        sug_thresh /= 1.1f;
        break;
    case Qt::Key_4:
        sug_thresh *= 1.1f;
        break;
    case Qt::Key_2:
        draw_hidden = !draw_hidden;
        break;
    case Qt::Key_6:
        clearMesh();
        break;

    default:
        QGLWidget::keyPressEvent(e);
    }
    updateGL();
}

void LineDrawingWidget::keyReleaseEvent(QKeyEvent * /*e*/)
{
    isCtrlPressed = false;
}

//---------------------------------private function------------------------------

void LineDrawingWidget::reset()
{
    if(!themesh)
        return;

    if(!xf.read(xfFileName))
        xf = xform::trans(0, 0, -5.0f * themesh->bsphere.r)*xform::trans(-themesh->bsphere.center);
}
