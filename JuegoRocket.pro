QT += core gui widgets multimedia

CONFIG += c++17
CONFIG -= debug_and_release
TEMPLATE = app
TARGET = JuegoRocket
DESTDIR = $$PWD/bin

INCLUDEPATH += $$PWD/src $$PWD/src/logica $$PWD/src/gui

RCC_EXE = $$[QT_INSTALL_BINS]/rcc.exe
RECURSO_BIN = $$PWD/bin/recursos.rcc
recursos_bin.target = $$RECURSO_BIN
recursos_bin.depends = $$PWD/recursos.qrc
recursos_bin.commands = $$shell_quote($$RCC_EXE) -binary $$shell_quote($$PWD/recursos.qrc) -o $$shell_quote($$RECURSO_BIN)
QMAKE_EXTRA_TARGETS += recursos_bin
PRE_TARGETDEPS += $$RECURSO_BIN

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
    src/gui/HojaSprites.h \
    src/gui/MenuInicio.h \
    src/gui/EscenaMision.h \
    src/gui/Marcador.h \
    src/gui/EscenaEmboscada.h \
    src/gui/EscenaAsalto.h \
    src/gui/EscenaPenon.h \
    src/gui/Sonido.h \
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
    src/gui/HojaSprites.cpp \
    src/gui/MenuInicio.cpp \
    src/gui/Marcador.cpp \
    src/gui/EscenaEmboscada.cpp \
    src/gui/EscenaAsalto.cpp \
    src/gui/EscenaPenon.cpp \
    src/gui/Sonido.cpp \
    src/gui/VentanaJuego.cpp
