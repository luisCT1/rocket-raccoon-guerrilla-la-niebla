#ifndef VENTANA_JUEGO_H
#define VENTANA_JUEGO_H

#include <QMainWindow>
#include <QSet>
#include <QKeyEvent>

class QLabel;
class QTimer;
class Juego;
class Rocket;

//Ventana principal del juego (por ahora HUD de texto mientras armamos la escena)

class VentanaJuego : public QMainWindow {
    Q_OBJECT

public:
    explicit VentanaJuego(QWidget *parent = nullptr);
    ~VentanaJuego() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void tick();

private:
    void actualizarHud();
    Rocket *rocketActivo() const;

    Juego *juego;
    QTimer *reloj;
    QLabel *hud;
    QSet<int> teclas;
};

#endif
