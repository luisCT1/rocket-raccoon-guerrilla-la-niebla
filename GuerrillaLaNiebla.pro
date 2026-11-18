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
    src/logica/MovOscilatorio.h \
    src/logica/MovCircular.h \
    src/logica/Entidad.h \
    src/logica/Proyectil.h \
    src/logica/Rocket.h \
    src/logica/Enemigo.h \
    src/logica/Estructura.h \
    src/logica/Nivel.h \
    src/logica/NivelEmboscada.h \
    src/logica/NivelAsalto.h \
    src/logica/NivelPenon.h \
    src/logica/Eventos.h \
    src/logica/AgenteTipos.h \
    src/logica/Capitan.h \
    src/logica/Juego.h \
    src/gui/Animacion.h \
    src/gui/HojaSprites.h \
    src/gui/MenuInicio.h \
    src/gui/EscenaEmboscada.h \
    src/gui/VentanaJuego.h

SOURCES += \
    src/main.cpp \
    src/logica/ModeloFisico.cpp \
    src/logica/MovParabolico.cpp \
    src/logica/MovOscilatorio.cpp \
    src/logica/MovCircular.cpp \
    src/logica/Entidad.cpp \
    src/logica/Proyectil.cpp \
    src/logica/Rocket.cpp \
    src/logica/Enemigo.cpp \
    src/logica/Estructura.cpp \
    src/logica/Nivel.cpp \
    src/logica/NivelEmboscada.cpp \
    src/logica/NivelAsalto.cpp \
    src/logica/NivelPenon.cpp \
    src/logica/Capitan.cpp \
    src/logica/Juego.cpp \
    src/gui/Animacion.cpp \
    src/gui/HojaSprites.cpp \
    src/gui/MenuInicio.cpp \
    src/gui/EscenaEmboscada.cpp \
    src/gui/VentanaJuego.cpp
