/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef OGL_WIDGET_H
#define OGL_WIDGET_H

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QWidget>
#include "../../../src/types.h"

template <typename T> struct image_s;

class OGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit OGLWidget(QWidget *parent = 0);

    void upload_canvas_texture(const image_s<u8> &image);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    // The index of the OpenGL texture into which we'll copy the frame buffer
    // for rendering.
    GLuint frameBufferTextureIdx = 0;
};

#endif
