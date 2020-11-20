/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#include <QMouseEvent>
#include <QApplication>
#include <QPainter>
#include <QEvent>
#include "../../src/display/w_opengl.h"
#include "../../src/display/window.h"
#include "../../src/common.h"
#include "../../src/image.h"
#include "../../src/main.h"
#include "../../src/ui.h"

static OGLWidget *OGL_SURFACE = nullptr;

static QImage CANVAS;

// The amount by which the mouse cursor has moved since the last event update.
static QPoint MOUSE_MOVE_DELTA;

Window::Window(const uint width, const uint height, const char *const title)
{
    // Initialize the window.
    this->setAttribute(Qt::WA_OpaquePaintEvent, true);  // We'll repaint the entire window every time.
    this->setMouseTracking(true);
    this->installEventFilter(this);
    this->resize(width, height);
    this->setFixedSize(size());

    this->setWindowTitle(title);

    // Initialize OpenGL.
    OGL_SURFACE = new OGLWidget(this);
    OGL_SURFACE->resize(this->size());
    OGL_SURFACE->setFocusPolicy(Qt::NoFocus); // Prevent the surface from receiving keyboard inputs, so they fall through into the main window's handler.
    OGL_SURFACE->setAttribute(Qt::WA_TransparentForMouseEvents);    // Let mouse input pass through the OpenGL surface and into the main window's event handler.

    this->show();

    return;
}

Window::~Window()
{
    return;
}

void Window::set_canvas_image(const image_s &image)
{
    k_assert(image.bpp() == 32, "Expected a 32-bit image.");

    OGL_SURFACE->upload_canvas_texture(image);
    OGL_SURFACE->update();

    return;
}

vector2_s<int> Window::mouse_move_delta(void)
{
    return {MOUSE_MOVE_DELTA.x(), MOUSE_MOVE_DELTA.y()};
}

void Window::reset_mouse_move_delta(void)
{
    MOUSE_MOVE_DELTA = QPoint(0, 0);

    return;
}

bool Window::eventFilter(QObject *, QEvent *event)
{
    static QPoint prevMousePos = QCursor::pos();

    if (event->type() == QEvent::MouseMove)
    {
        const QMouseEvent *const e = (QMouseEvent*)event;

        if (QApplication::mouseButtons() & Qt::LeftButton)
        {
            MOUSE_MOVE_DELTA = (e->pos() - prevMousePos);
        }

        prevMousePos = e->pos();

        return true;
    }
    else if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *e = (QKeyEvent*)event;

        switch (e->key())
        {
            /// Temp hack for movement. Good enough for testing, but will be replaced later.
            case Qt::Key_E: { kinput_move_camera_forward(); break; }
            case Qt::Key_D: { kinput_move_camera_backward(); break; }
            default: break;
        }

        return true;
    }

    return false;
}

void Window::closeEvent(QCloseEvent *)
{
    DEBUG(("Have been asked to close the window."));

    kmain_request_program_exit(EXIT_SUCCESS);

    return;
}
