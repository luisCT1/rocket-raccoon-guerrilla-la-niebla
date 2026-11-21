#ifndef ESCENA_PENON_H
#define ESCENA_PENON_H

#include "EscenaMision.h"
#include "Marcador.h"

#include <QHash>
#include <QImage>
#include <QPixmap>
#include <QPoint>
#include <QSet>
#include <QVector>

class QGraphicsEllipseItem;
class QGraphicsItem;
class QGraphicsPixmapItem;
class Entidad;
class Juego;
class NivelPenon;
class Sonido;

class EscenaPenon : public EscenaMision {
public:
    EscenaPenon();
    ~EscenaPenon() override;

    void pulsar(int tecla) override;
    void soltar(int tecla) override;
    void mantener(int tecla) override;
    void apuntar(float x, float y) override;
    void avanzar(float dt) override;

    QRectF rectCamara() const override;
    QString textoHud() const override;
    QString mensajeFinal() const override;
    bool gano() const override;
    bool enFuria() const override;
    bool acabo() const override;

private:
    struct Capa {
        QGraphicsItem *dibujo = nullptr;
        float x = 0.f;
        float y = 0.f;
        float avance = 1.f;
        float deriva = 0.f;
        float ciclo = 0.f;
    };

    struct Estallido {
        QGraphicsPixmapItem *item = nullptr;
        float edad = 0.f;
        float x = 0.f;
        float y = 0.f;
    };

    void cargarSprites();
    void prepararHud(const QPixmap &hoja);
    void armarMundo();
    void moverCapas();
    void sincronizar();
    void dibujarHud();
    void apoyar(QGraphicsPixmapItem *sprite, const QPixmap &cuadro, float centro, float suelo, float alto, float altoFuente = 0.f, float fraccion = 0.55f);
    void apoyarSuperficie(QGraphicsPixmapItem *sprite, const QPixmap &cuadro, float izquierda, float superficie, float ancho);
    QPixmap cuadroDe(Entidad *entidad) const;
    bool miraALaIzquierda(Entidad *entidad) const;

    Juego *juego = nullptr;
    NivelPenon *nivel = nullptr;

    QVector<Capa> capas;
    QHash<Entidad *, QGraphicsItem *> piezas;
    QHash<Entidad *, QGraphicsEllipseItem *> avisos;
    QVector<Estallido> estallidos;
    QVector<QGraphicsItem *> flashes;
    QVector<float> vidaFlash;

    QVector<QPixmap> caminar;
    QVector<QPixmap> espera;
    QVector<QPixmap> saltar;
    QVector<QPixmap> disparar;
    QVector<QPixmap> impulso;
    QVector<QPixmap> furia;
    QVector<QPixmap> agachar;
    QVector<QPixmap> lanzar;
    QVector<QPixmap> rocketDanio;
    QVector<QPixmap> rocketMuerte;

    QVector<QPixmap> capitanCamina;
    QVector<QPixmap> capitanEspera;
    QVector<QPixmap> capitanAcosa;
    QVector<QPixmap> capitanCanon;
    QVector<QPixmap> capitanGiro;
    QVector<QPixmap> capitanTromba;
    QVector<QPixmap> capitanRepliegue;
    QVector<QPixmap> capitanMuerte;

    QVector<QPixmap> canonCuadros;
    QVector<QPixmap> balaCanon;
    QVector<QPixmap> explosion;
    QVector<QPixmap> banderaCuadros;
    QVector<QPixmap> viento;

    QVector<QPixmap> suelos;
    QVector<QPixmap> losas;
    QPixmap arco;
    QPixmap ruina;
    QPixmap arbolCima;
    QVector<QPixmap> iconos;
    QVector<QPixmap> arboles;
    QVector<QPixmap> helechos;
    QVector<QPixmap> rocas;

    PiezasHud piezasHud;
    QImage panelVidas;
    QVector<QImage> corazones;
    QVector<QPoint> posCorazon;
    QImage barraLlena;
    QImage barraVacia;
    QRect rojoBarra;
    QRect textoBarra;
    QImage balasLlenas;
    QImage balasVacias;
    QVector<QImage> iconosBala;
    QVector<QPoint> posBala;
    QRect textoBalas;
    QGraphicsPixmapItem *marcador = nullptr;
    QGraphicsPixmapItem *marcadorJefe = nullptr;
    QGraphicsPixmapItem *bandera = nullptr;
    QGraphicsEllipseItem *meta = nullptr;
    QGraphicsPixmapItem *rafaga = nullptr;
    QGraphicsPixmapItem *tajo = nullptr;
    QPixmap tajoCuadro;

    QSet<int> teclas;
    float tiempo = 0.f;
    float granadaMuestra = 0.f;
    float canonPulso = 0.f;
    float avisoTiempo = 0.f;
    float edadMuerte = 0.f;
    float capitanX = 0.f;
    short faseVista = 1;
    QString avisoFase;
    Sonido *sonidos = nullptr;
};

#endif
