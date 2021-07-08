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
#include "auxiliary/display/qt/window.h"
#include "vond/render.h"
#include "vond/camera.h"
#include "vond/assert.h"

// The window we'll display the program in.
static Window *WINDOW = nullptr;

// How many times per second the display is being updated.
static unsigned FPS = 0;

// For the Qt GUI to work, we need to have a QApplication object around. We just
// pass some dummy args to it.
namespace app_n
{
    static char NAME[] = "Vond";
    static int ARGC = 1;
    static char *ARGV = NAME;
    static QApplication *const APP = new QApplication(ARGC, &ARGV);
}

void kd_update_display(const vond::image<uint8_t, 4> &pixelmap)
{
    static uint32_t fpsCnt = 0;

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

    WINDOW->set_canvas_image(pixelmap);
    WINDOW->update();

    // Spin the event loop manually, relying on OpenGL's refresh block to keep us
    // in sync with the monitor's refresh rate.
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents();

    return;
}

void kd_update_input(vond::camera *const camera)
{
    // Rotate the camera's viewing direction.
    {
        const double rotSpeed = 0.003;
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

void kd_acquire_display(const unsigned width, const unsigned height, const char *const title)
{
    printf("Acquiring the display...\n");

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
    printf("Freeing the display.\n");

    delete WINDOW;
    delete app_n::APP;

    return;
}
