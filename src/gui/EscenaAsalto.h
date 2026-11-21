#ifndef ESCENA_ASALTO_H
#define ESCENA_ASALTO_H

#include "EscenaMision.h"
#include "Marcador.h"

#include <QHash>
#include <QPixmap>
#include <QSet>
#include <QVector>

class QGraphicsEllipseItem;
class QGraphicsPixmapItem;
class QGraphicsSimpleTextItem;
class Entidad;
class Juego;
class NivelAsalto;

class EscenaAsalto : public EscenaMision {
public:
    EscenaAsalto();
    ~EscenaAsalto() override;

    void pulsar(int tecla) override;
    void soltar(int tecla) override;
    void apuntar(float x, float y) override;
    void avanzar(float dt) override;

    QRectF rectCamara() const override;
    QString textoHud() const override;
    QString mensajeFinal() const override;
    bool gano() const override;
    bool acabo() const override;

private:
    void cargarSprites();
    void armarPatio();
    void sincronizar();
    void dibujarHud();
    void colocar(Entidad *entidad, QGraphicsPixmapItem *sprite);
    QPixmap cuadroDe(Entidad *entidad) const;

    Juego *juego = nullptr;
    NivelAsalto *nivel = nullptr;

    QHash<Entidad *, QGraphicsPixmapItem *> piezas;
    QVector<QPixmap> caminar;
    QVector<QPixmap> disparar;
    QVector<QPixmap> lanzar;
    QVector<QPixmap> impulso;
    QVector<QPixmap> furia;
    QVector<QPixmap> rocketMuerte;
    QVector<QPixmap> soldadoCamina;
    QVector<QPixmap> soldadoDispara;

    QPixmap polvorin;
    QPixmap polvorinRoto;
    QPixmap bandera;
    QPixmap banderaCaida;
    QPixmap canon;
    QPixmap canonRoto;
    QPixmap torre;

    PiezasHud piezasHud;
    QGraphicsPixmapItem *marcador = nullptr;
    QGraphicsSimpleTextItem *reloj = nullptr;
    QGraphicsSimpleTextItem *tareas = nullptr;
    QGraphicsEllipseItem *salida = nullptr;

    QSet<int> teclas;
    float tiempo = 0.f;
    float granadaMuestra = 0.f;
    float camX = 0.f;
    float camY = 0.f;
    float punteroX = 0.f;
    float punteroY = 0.f;
    bool hayApunte = false;
};

#endif
