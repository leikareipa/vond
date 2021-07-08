/*
 * Tarpeeksi Hyvae Soft 2018 /
 * Vond
 *
 */

#ifndef D_WINDOW_H
#define D_WINDOW_H

#include <QWidget>
#include "vond/vector.h"
#include "vond/image.h"

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(const unsigned width, const unsigned height, const char *const title);
    ~Window();

    void set_canvas_image(const vond::image<uint8_t, 4> &image);

    vond::vector2<int> mouse_move_delta();

    void reset_mouse_move_delta();

private:
    void closeEvent(QCloseEvent*);
    bool eventFilter(QObject*, QEvent *event);
    //void paintEvent(QPaintEvent *);
};

#endif
