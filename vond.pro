DEFINES += ENFORCE_OPTIONAL_ASSERTS # Enable non-critical asserts. May perform slower, but will e.g. look to guard against buffer overflow in memory access.

TEMPLATE = app
QT       += core gui widgets
CONFIG   += console c++11

OBJECTS_DIR = generated_files
MOC_DIR = generated_files
UI_DIR = generated_files

SOURCES += src/main.cpp \
    src/display/display.cpp \
    src/display/window.cpp \
    src/render/landscape.cpp \
    src/memory/memory.cpp \
    src/display/w_opengl.cpp \
    src/ui/text.cpp \
    src/render/render.cpp \
    src/ui/input.cpp \
    src/render/triangle.cpp \
    src/data_access/mesh_file.cpp \
    src/data_access/world_file.cpp \
    src/data_access/config_file_read.cpp

HEADERS += \
    src/display.h \
    src/camera.h \
    src/types.h \
    src/display/window.h \
    src/common.h \
    src/main.h \
    src/render/landscape.h \
    src/memory.h \
    src/memory/memory_interface.h \
    src/image.h \
    src/matrix44.h \
    src/render.h \
    src/display/w_opengl.h \
    src/ui.h \
    src/ui/text.h \
    src/vector.h \
    src/ui/input.h \
    src/render/polygon.h \
    src/data_access/mesh_file.h \
    src/data_access/world_file.h \
    src/config_file_read.h \
    src/data_access/asset_store.h

# C++. For GCC/Clang.
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS += -ansi
QMAKE_CXXFLAGS += -O2
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -pipe
QMAKE_CXXFLAGS += -pedantic

QMAKE_CXXFLAGS += -fopenmp
LIBS += -fopenmp
