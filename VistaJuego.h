#ifndef VISTAJUEGO_H
#define VISTAJUEGO_H

#include <QElapsedTimer>
#include <QPointF>
#include <QSet>
#include <QTimer>
#include <QWidget>
#include <vector>

#include "Juego.h"

class Sprites;
class Sonido;
class Entidad;
class Nivel;

class VistaJuego : public QWidget {
    Q_OBJECT
private:
    enum Estado { JUGANDO, PAUSA, FIN };

    Juego juego;
    Sprites *sprites;
    Sonido *sonido;
    QTimer reloj;
    QElapsedTimer cronometro;
    QSet<int> teclas;
    std::vector<short> eventos;
    Estado estado;
    bool campania;
    bool botonIzq;
    QPointF raton;

    float esc, offX, offY, visAncho, visAlto;

    void calcularVista();
    QPointF aMundo(const QPointF &p) const;
    void procesarEntrada();
    void cuadro();

    void dibujarMundo(QPainter &p);
    void dibujarFondo(QPainter &p, Nivel *n);
    void dibujarEntidad(QPainter &p, const Entidad *e);
    void dibujarExtras(QPainter &p, Nivel *n);
    void dibujarHUD(QPainter &p, Nivel *n);
    void dibujarOverlay(QPainter &p, const QString &titulo, const QString &subtitulo);
    void dibujarSprite(QPainter &p, const QString &nombre, const QRectF &r, bool voltear,
                       const QColor &color, bool redondo = false);

public:
    VistaJuego(Sprites *sp, Sonido *so, QWidget *padre = nullptr);

    void iniciar(short nivel, short dificultad, bool esCampania);
    void detener();

signals:
    void volverAlMenu();

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
};

#endif
