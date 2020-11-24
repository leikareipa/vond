DEFINES += ENFORCE_OPTIONAL_ASSERTS # Enable non-critical asserts. May perform slower, but will e.g. look to guard against buffer overflow in memory access.

TEMPLATE = app
QT       += core gui widgets
CONFIG   += console

OBJECTS_DIR = generated_files
MOC_DIR = generated_files
UI_DIR = generated_files

INCLUDEPATH += $$PWD/src/

SOURCES += src/auxiliary/main.cpp \
    src/auxiliary/display/qt/display.cpp \
    src/auxiliary/display/qt/window.cpp \
    src/vond/rasterizer_barycentric.cpp \
    src/vond/rasterizer_scanline.cpp \
    src/vond/render_landscape.cpp \
    src/auxiliary/display/qt/w_opengl.cpp \
    src/auxiliary/ui/text.cpp \
    src/auxiliary/ui/input.cpp \
    src/vond/render_triangles.cpp \
    src/auxiliary/data_access/mesh_file.cpp \
    src/auxiliary/data_access/config_file_read.cpp

HEADERS += \
    src/auxiliary/display.h \
    src/vond/camera.h \
    src/auxiliary/display/qt/window.h \
    src/vond/assert.h \
    src/auxiliary/main.h \
    src/vond/rasterizer_barycentric.h \
    src/vond/rasterizer_scanline.h \
    src/vond/rect.h \
    src/vond/render_landscape.h \
    src/vond/image.h \
    src/vond/matrix.h \
    src/vond/render.h \
    src/auxiliary/display/qt/w_opengl.h \
    src/auxiliary/ui.h \
    src/auxiliary/ui/text.h \
    src/vond/triangle.h \
    src/vond/vector.h \
    src/auxiliary/ui/input.h \
    src/vond/render_triangles.h \
    src/auxiliary/data_access/mesh_file.h \
    src/auxiliary/config_file_read.h \
    src/vond/vertex.h

# C++. For GCC/Clang.
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -std=c++2a
QMAKE_CXXFLAGS += -O2
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -pipe
QMAKE_CXXFLAGS += -pedantic

QMAKE_CXXFLAGS += -fopenmp
LIBS += -fopenmp
