#DEFINES += ENFORCE_OPTIONAL_ASSERTS # Enable non-critical asserts. May perform slower, but will e.g. look to guard against buffer overflow in memory access.

TEMPLATE = app
QT       += core gui widgets
CONFIG   += console c++20

OBJECTS_DIR = generated_files
MOC_DIR = generated_files
UI_DIR = generated_files

INCLUDEPATH += $$PWD/src/

SOURCES += src/auxiliary/main.cpp \
    src/auxiliary/display/qt/display.cpp \
    src/auxiliary/display/qt/window.cpp \
    src/vond/rasterize_triangle_barycentric.cpp \
    src/vond/rasterize_triangle_scanline.cpp \
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
    src/vond/color.h \
    src/vond/image_mosaic.h \
    src/vond/rasterize_triangle_barycentric.h \
    src/vond/rasterize_triangle_scanline.h \
    src/vond/ray.h \
    src/vond/rect.h \
    src/vond/render_landscape.h \
    src/vond/image.h \
    src/vond/matrix.h \
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

contains(DEFINES, INCLUDE_HOSEK_WILKIE_SKY) {
    SOURCES += src/auxiliary/hosek-wilkie-sky/ArHosekSkyModel.cpp

    HEADERS += src/auxiliary/hosek-wilkie-sky/ArHosekSkyModel.h \
               src/auxiliary/hosek-wilkie-sky/ArHosekSkyModelData_CIEXYZ.h \
               src/auxiliary/hosek-wilkie-sky/ArHosekSkyModelData_RGB.h \
               src/auxiliary/hosek-wilkie-sky/ArHosekSkyModelData_Spectral.h
}

QMAKE_CC = gcc-10
QMAKE_CXX = g++-10
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -std=c++20
QMAKE_CXXFLAGS += -O2
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -pipe
QMAKE_CXXFLAGS += -pedantic
QMAKE_CXXFLAGS += -fopenmp
LIBS += -fopenmp
