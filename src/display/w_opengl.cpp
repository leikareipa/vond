/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 * Uses Qt's OpenGL implementation to draw the contents of RGEO's render framebuffer
 * to screen. I.e. just draws a full-window textured quad.
 *
 */

#include <QOpenGLFunctions>
#include <QCoreApplication>
#include <QOpenGLWidget>
#include <QMatrix4x4>
#include <QPainter>
#include "../../../src/display/w_opengl.h"
#include "../../../src/display.h"
#include "../../../src/render.h"
#include "../../../src/image.h"
#include "../../../src/ui.h"

OGLWidget::OGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    return;
}

void OGLWidget::initializeGL()
{
    DEBUG(("Initializing OpenGL..."));

    this->initializeOpenGLFunctions();

    DEBUG(("OpenGL is reported to be version '%s'.", glGetString(GL_VERSION)));

    this->setUpdateBehavior(QOpenGLWidget::PartialUpdate);
    this->glDisable(GL_DEPTH_TEST);
    this->glEnable(GL_TEXTURE_2D);

    // Initialize the texture into which we'll stream the renderer's framebuffer.
    this->glGenTextures(1, &frameBufferTextureIdx);
    this->glBindTexture(GL_TEXTURE_2D, frameBufferTextureIdx);
    this->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    this->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    k_assert(!glGetError(), "OpenGL initialization failed.");

    return;
}

void OGLWidget::resizeGL(int w, int h)
{
    QMatrix4x4 m;
    m.setToIdentity();
    m.ortho(0, w, h, 0, -1, 1);

    glLoadMatrixf(m.constData());

    return;
}

void OGLWidget::paintGL()
{
    // Draw a textured full-screen quad that contains the current render framebuffer.
    {
        this->glBindTexture(GL_TEXTURE_2D, frameBufferTextureIdx);

        glBegin(GL_TRIANGLES);
            glTexCoord2i(0, 0); glVertex2i(0,             0);
            glTexCoord2i(0, 1); glVertex2i(0,             this->height());
            glTexCoord2i(1, 1); glVertex2i(this->width(), this->height());

            glTexCoord2i(1, 1); glVertex2i(this->width(), this->height());
            glTexCoord2i(1, 0); glVertex2i(this->width(), 0);
            glTexCoord2i(0, 0); glVertex2i(0,             0);
        glEnd();
    }

    // Draw any UI text we might have.
    {
        QPainter painter(this);

        auto uiText = ktext_ui_text_entries();
        for (const auto &text: uiText)
        {
            painter.drawText(text.coords.x, text.coords.y, text.text.c_str());
        }
    }

    return;
}

void OGLWidget::upload_canvas_texture(const image_s<u8> &image)
{
    k_assert_optional(image.pixel_array(), "Expected a non-null pixel array.");

    this->glBindTexture(GL_TEXTURE_2D, frameBufferTextureIdx);
    this->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.pixel_array());

    return;
}

