#ifndef MENU_INICIO_H
#define MENU_INICIO_H

#include "Eventos.h"

#include <QWidget>

class QPushButton;

class MenuInicio : public QWidget {
    Q_OBJECT

public:
    explicit MenuInicio(QWidget *parent = nullptr);

    short dificultad() const;

protected:
    void paintEvent(QPaintEvent *evento) override;

signals:
    void campana();
    void nivelElegido(int numero);
    void salir();
    void dificultadCambiada();

private:
    void actualizarTextoDificultad();

    short dificultadActual = DIF_NORMAL;
    QPushButton *botonDificultad = nullptr;
    QPushButton *botonCampana = nullptr;
    QPushButton *botonEmboscada = nullptr;
    QPushButton *botonAsalto = nullptr;
    QPushButton *botonPenon = nullptr;
    QPushButton *botonSalir = nullptr;
};

#endif
