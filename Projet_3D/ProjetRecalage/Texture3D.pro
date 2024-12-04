# -------------------------------------------------
# Project created by QtCreator 2010-01-27T15:21:45
# -------------------------------------------------
QT += xml
QT += opengl
TARGET = texture3D
TEMPLATE = app
MOC_DIR = ./moc
OBJECTS_DIR = ./obj
DEPENDPATH += ./GLSL
INCLUDEPATH += ./GLSL \
    CImg.h
SOURCES += Main.cpp \
    Window.cpp \
    TextureViewer.cpp \
    Texture.cpp \
    TextureDockWidget.cpp
HEADERS += Window.h \
    TextureViewer.h \
    Texture.h \
    TextureDockWidget.h \
    Vec3D.h \
    Img.h
INCLUDEPATH = ./GLSL
LIBS = -lQGLViewer-qt5 \
    -lglut \
    -lGLU \
    -lX11 \
    -L/usr/lib/x86_64-linux-gnu -lQt5Core
