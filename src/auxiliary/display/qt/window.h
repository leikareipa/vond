/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef D_WINDOW_H
#define D_WINDOW_H

#include <QWidget>
#include "vond/vector.h"

template <typename T> struct image_s;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(const unsigned width, const unsigned height, const char *const title);
    ~Window();

    void set_canvas_image(const image_s<uint8_t> &image);

    vector2_s<int> mouse_move_delta();

    void reset_mouse_move_delta();

private:
    void closeEvent(QCloseEvent*);
    bool eventFilter(QObject*, QEvent *event);
    //void paintEvent(QPaintEvent *);
};

#endif
