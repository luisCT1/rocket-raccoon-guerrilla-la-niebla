#ifndef ESCENA_EMBOSCADA_H
#define ESCENA_EMBOSCADA_H

#include "EscenaMision.h"
#include "Marcador.h"

#include <QHash>
#include <QImage>
#include <QPixmap>
#include <QPoint>
#include <QSet>
#include <QVector>

class QGraphicsItem;
class QGraphicsPixmapItem;
class Entidad;
class Juego;
class NivelEmboscada;

class EscenaEmboscada : public EscenaMision {
public:
    EscenaEmboscada();
    ~EscenaEmboscada() override;

    void pulsar(int tecla) override;
    void soltar(int tecla) override;
    void apuntar(float x, float y) override;
    void avanzar(float dt) override;

    QRectF rectCamara() const override;
    QString textoHud() const override;
    QString mensajeFinal() const override;
    int porcentajeVida() const;
    bool acabo() const override;
    bool gano() const override;

private:
    struct CapaFondo {
        QGraphicsItem *dibujo = nullptr;
        float x = 0.f;
        float y = 0.f;
        float avance = 1.f;
        float deriva = 0.f;
        float ciclo = 0.f;
    };

    struct Caida {
        QGraphicsPixmapItem *item = nullptr;
        QVector<QPixmap> cuadros;
        float edad = 0.f;
        float centro = 0.f;
        float suelo = 0.f;
        float alto = 96.f;
        float fuente = 0.f;
    };

    void cargarSprites();
    void prepararHud(const QPixmap &hoja);
    void dibujarHud();
    void armarSuelo();
    void moverCapas();
    void sincronizar();
    void colocar(Entidad *entidad, QGraphicsItem *dibujo);
    void apoyar(QGraphicsPixmapItem *sprite, const QPixmap &cuadro, float centro, float suelo, float alto, float altoFuente = 0.f, float fraccion = 0.55f);
    QPixmap cuadroDe(Entidad *entidad) const;
    bool miraALaIzquierda(Entidad *entidad) const;

    Juego *juego = nullptr;
    NivelEmboscada *nivel = nullptr;

    QVector<CapaFondo> capas;
    QHash<Entidad *, QGraphicsItem *> piezas;
    QVector<QGraphicsItem *> flashes;
    QVector<float> vidaFlash;

    QVector<QPixmap> caminar;
    QVector<QPixmap> espera;
    QVector<QPixmap> saltar;
    QVector<QPixmap> disparar;
    QVector<QPixmap> impulso;
    QVector<QPixmap> furia;
    QVector<QPixmap> lanzar;
    QVector<QPixmap> soldadoCamina;
    QVector<QPixmap> soldadoDispara;
    QVector<QPixmap> soldadoBandera;
    QVector<QPixmap> rocketDanio;
    QVector<QPixmap> rocketMuerte;
    QVector<QPixmap> soldadoMuere;
    QVector<QPixmap> jineteGalope;
    QVector<QPixmap> jineteBandera;
    QVector<QPixmap> jineteTrompeta;
    QVector<QPixmap> jineteMuere;
    QVector<QPixmap> carretaRueda;
    QVector<QPixmap> carretaHerida;
    QVector<QPixmap> carretaExplota;
    QVector<QPixmap> barrilQuieto;
    QVector<QPixmap> barrilEstalla;
    QPixmap puenteQuieto;

    PiezasHud piezasHud;
    QGraphicsPixmapItem *marcador = nullptr;
    QVector<Caida> caidas;

    QSet<int> teclas;
    float tiempo = 0.f;
    float granadaMuestra = 0.f;
    float punteroX = 0.f;
    float punteroY = 0.f;
    bool hayApunte = false;
};

#endif
