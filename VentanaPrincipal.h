#ifndef VENTANAPRINCIPAL_H
#define VENTANAPRINCIPAL_H

#include <QStackedWidget>

class MenuWidget;
class VistaJuego;
class Sprites;
class Sonido;

class VentanaPrincipal : public QStackedWidget {
    Q_OBJECT
private:
    MenuWidget *menu;
    VistaJuego *vista;
    Sprites *sprites;
    Sonido *sonido;

    void mostrarMenu();

public:
    VentanaPrincipal();
    ~VentanaPrincipal() override;
};

#endif
