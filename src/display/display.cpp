/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#include <QSurfaceFormat>
#include <QElapsedTimer>
#include <QApplication>
#include <QMouseEvent>
#include <QMessageBox>
#include <QPainter>
#include <QWidget>
#include <QDebug>
#include "../../src/display/window.h"
#include "../../src/render.h"
#include "../../src/camera.h"
#include "../../src/common.h"

// The window we'll display the program in.
static Window *WINDOW = nullptr;

// How many times per second the display is being updated.
static uint FPS = 0;

// For the Qt GUI to work, we need to have a QApplication object around. We just
// pass some dummy args to it.
namespace app_n
{
    static char NAME[] = "Vond";
    static int ARGC = 1;
    static char *ARGV = NAME;
    static QApplication *const APP = new QApplication(ARGC, &ARGV);
}

void kd_update_display(const image_s &frameBufferCanvas)
{
    static u32 fpsCnt = 0;

    static QElapsedTimer fpsTimer;
    if (!fpsTimer.isValid())
    {
        fpsTimer.start();
    }

    fpsCnt++;
    if (fpsTimer.elapsed() >= 1000)
    {
        FPS = fpsCnt;
        fpsCnt = 0;
        fpsTimer.restart();
    }

    WINDOW->set_canvas_image(frameBufferCanvas);
    WINDOW->update();

    // Spin the event loop manually, relying on OpenGL's refresh block to keep us
    // in sync with the monitor's refresh rate.
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents();

    return;
}

void kd_update_input(camera_s *const camera)
{
    // Rotate the camera's viewing direction.
    {
        const real rotSpeed = 0.003;
        const auto mouseDelta = WINDOW->mouse_move_delta();

        camera->orientation.x -= (mouseDelta.y * rotSpeed);
        camera->orientation.y += (mouseDelta.x * rotSpeed);

        if (fabs(camera->orientation.y) > (M_PI * 2))
        {
            camera->orientation.y = 0;
        }

        WINDOW->reset_mouse_move_delta();
    }

    return;
}

void kd_acquire_display(const uint width, const uint height, const char *const title)
{
    DEBUG(("Acquiring the display."));

    // Set the OpenGL surface format.
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(1, 2);
    format.setSwapInterval(1); // Vsync.
    format.setSamples(0);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);

    WINDOW = new Window(width, height, title);

    return;
}

void kd_release_display(void)
{
    DEBUG(("Freeing the display."));

    delete WINDOW;
    delete app_n::APP;

    return;
}
