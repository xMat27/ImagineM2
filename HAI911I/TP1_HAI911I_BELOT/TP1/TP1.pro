MOC_DIR = ./moc
OBJECTS_DIR = ./obj

INCLUDEPATH += $$PWD

HEADERS       = glwidget.h \
                loadOFF.h \
                mesh.h \
                window.h \
                mainwindow.h \
                logo.h

SOURCES       = glwidget.cpp \
                loadOFF.cpp \
                main.cpp \
                mesh.cpp \
                window.cpp \
                mainwindow.cpp \
                logo.cpp


RESOURCES += \
    shaders.qrc

QT           += widgets \
                opengl



