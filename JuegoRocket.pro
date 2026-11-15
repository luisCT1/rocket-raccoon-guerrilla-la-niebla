QT += core gui widgets multimedia
CONFIG += c++17
TARGET = JuegoRocket
TEMPLATE = app

HEADERS += \
Accion.h \
AgenteTipos.h \
Animacion.h \
Aprendizaje.h \
Capitan.h \
Dificultad.h \
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
Percepcion.h \
Proyectil.h \
Razonamiento.h \
Rocket.h \
Sonido.h \
Sprites.h \
VentanaPrincipal.h \
VistaJuego.h

SOURCES += \
Accion.cpp \
Animacion.cpp \
Aprendizaje.cpp \
Capitan.cpp \
Dificultad.cpp \
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
Percepcion.cpp \
Proyectil.cpp \
Razonamiento.cpp \
Rocket.cpp \
Sonido.cpp \
Sprites.cpp \
VentanaPrincipal.cpp \
VistaJuego.cpp \
main.cpp

# Carpeta con sprites/ y audio/
DEFINES += RUTA_RECURSOS=\\\"$$PWD/recursos/\\\"
