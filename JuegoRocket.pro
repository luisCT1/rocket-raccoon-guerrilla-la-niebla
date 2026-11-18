QT += core gui widgets multimedia
CONFIG += c++17
TARGET = JuegoRocket
TEMPLATE = app

HEADERS += \
AgenteTipos.h \
Animacion.h \
Capitan.h \
Enemigo.h \
Entidad.h \
Estructura.h \
Eventos.h \
Juego.h \
MenuWidget.h \
ModeloFisico.h \
MovCircular.h \
MovOscilatorio.h \
MovParabolico.h \
Nivel.h \
NivelAsalto.h \
NivelEmboscada.h \
NivelPenon.h \
Proyectil.h \
Rocket.h \
Sonido.h \
Sprites.h \
VentanaPrincipal.h \
VistaJuego.h

SOURCES += \
Animacion.cpp \
Capitan.cpp \
Enemigo.cpp \
Entidad.cpp \
Estructura.cpp \
Juego.cpp \
MenuWidget.cpp \
ModeloFisico.cpp \
MovCircular.cpp \
MovOscilatorio.cpp \
MovParabolico.cpp \
Nivel.cpp \
NivelAsalto.cpp \
NivelEmboscada.cpp \
NivelPenon.cpp \
Proyectil.cpp \
Rocket.cpp \
Sonido.cpp \
Sprites.cpp \
VentanaPrincipal.cpp \
VistaJuego.cpp \
main.cpp

# Carpeta con sprites/ y audio/
DEFINES += RUTA_RECURSOS=\\\"$$PWD/recursos/\\\"
