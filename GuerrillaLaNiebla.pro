QT += core gui widgets

CONFIG += c++17
CONFIG -= debug_and_release
TEMPLATE = app
TARGET = GuerrillaLaNiebla
DESTDIR = $$PWD/bin

INCLUDEPATH += $$PWD/src $$PWD/src/logica $$PWD/src/gui

RESOURCES += recursos.qrc

HEADERS += \
    src/logica/ModeloFisico.h \
    src/logica/MovParabolico.h \
    src/logica/Animacion.h \
    src/logica/Entidad.h \
    src/logica/Proyectil.h \
    src/logica/Rocket.h \
    src/logica/Enemigo.h \
    src/logica/Nivel.h \
    src/logica/NivelDemo.h \
    src/logica/Juego.h \
    src/gui/HojaSprites.h \
    src/gui/EscenaEmboscada.h \
    src/gui/VentanaJuego.h

SOURCES += \
    src/main.cpp \
    src/logica/ModeloFisico.cpp \
    src/logica/MovParabolico.cpp \
    src/logica/Animacion.cpp \
    src/logica/Entidad.cpp \
    src/logica/Proyectil.cpp \
    src/logica/Rocket.cpp \
    src/logica/Enemigo.cpp \
    src/logica/Nivel.cpp \
    src/logica/NivelDemo.cpp \
    src/logica/Juego.cpp \
    src/gui/HojaSprites.cpp \
    src/gui/EscenaEmboscada.cpp \
    src/gui/VentanaJuego.cpp
