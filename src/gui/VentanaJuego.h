#ifndef VENTANA_JUEGO_H
#define VENTANA_JUEGO_H

#include <QMainWindow>

class QGraphicsView;
class QLabel;
class QPushButton;
class QStackedWidget;
class QTimer;
class EscenaEmboscada;
class MenuInicio;

class VentanaJuego : public QMainWindow {
    Q_OBJECT

public:
    explicit VentanaJuego(QWidget *parent = nullptr);
    ~VentanaJuego() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void tick();
    void empezarNivel();
    void volverAlMenu();

private:
    void encuadrar();
    void soltarTeclas();

    QStackedWidget *paginas = nullptr;
    MenuInicio *menu = nullptr;
    QGraphicsView *vista = nullptr;
    QLabel *hud = nullptr;
    QLabel *ayuda = nullptr;
    QLabel *mision = nullptr;
    QLabel *aviso = nullptr;
    QPushButton *volver = nullptr;
    QTimer *reloj = nullptr;
    EscenaEmboscada *escena = nullptr;
    bool enNivel = false;
    float tiempoMision = 0.f;
};

#endif
