/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef D_WINDOW_H
#define D_WINDOW_H

#include <QWidget>
#include "../../src/vector.h"
#include "../../src/types.h"

template <typename T> struct image_s;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(const uint width, const uint height, const char *const title);
    ~Window();

    void set_canvas_image(const image_s<u8> &image);

    vector2_s<int> mouse_move_delta();

    void reset_mouse_move_delta();

private:
    void closeEvent(QCloseEvent*);
    bool eventFilter(QObject*, QEvent *event);
    //void paintEvent(QPaintEvent *);
};

#endif
