QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    MovCircular.cpp\
    Estructura.cpp\
    Accion.cpp\
    Aprendizaje.cpp\
    Percepcion.cpp\
    Razonamiento.cpp\
    capitan.cpp\
    NivelAsalto.cpp\
    NivelPenon.cpp\
    Dificultad.cpp

HEADERS += \
    mainwindow.h \
    ModeloFisico.h \
    MovCircular.h\
    Eventos.h\
    Juego.h\
    Nivel.h\
    Entidad.h\
    Estructura.h\
    Proyectil.h\
    Rocket.h\
    AgenteTipos.h\
    Accion.h\
    Aprendizaje.h\
    Percepcion.h\
    Razonamiento.h\
    capitan.h\
    enemigo.h\
    NivelAsalto.h\
    NivelPenon.h\
    Dificultad.h


FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../../Users/davis/OneDrive/Escritorio/‎‎‎/kali-linux/juego_paso1_MovCircular.zip
