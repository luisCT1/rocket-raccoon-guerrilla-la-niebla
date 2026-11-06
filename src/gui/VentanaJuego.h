#ifndef VENTANA_JUEGO_H
#define VENTANA_JUEGO_H

#include <QMainWindow>

class QGraphicsView;
class QLabel;
class QTimer;
class EscenaEmboscada;

//Ventana del nivel 1. El fondo, el puente y el mapache viven en la escena.

class VentanaJuego : public QMainWindow {
    Q_OBJECT

public:
    explicit VentanaJuego(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void tick();

private:
    void encuadrar();

    EscenaEmboscada *escena = nullptr;
    QGraphicsView *vista = nullptr;
    QLabel *ayuda = nullptr;
    QTimer *reloj = nullptr;
};

#endif
