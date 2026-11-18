#ifndef ESCENA_EMBOSCADA_H
#define ESCENA_EMBOSCADA_H

#include <QGraphicsScene>
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

class EscenaEmboscada : public QGraphicsScene {
public:
    EscenaEmboscada();
    ~EscenaEmboscada() override;

    void pulsar(int tecla);
    void soltar(int tecla);
    void apuntar(float x, float y);
    void avanzar(float dt);

    QRectF rectCamara() const;
    QString textoHud() const;
    QString mensajeFinal() const;
    int porcentajeVida() const;
    bool acabo() const;
    bool gano() const;

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
    };

    void cargarSprites();
    void prepararHud(const QPixmap &hoja);
    void dibujarHud();
    void armarSuelo();
    void moverCapas();
    void sincronizar();
    void colocar(Entidad *entidad, QGraphicsItem *dibujo);
    void apoyar(QGraphicsPixmapItem *sprite, const QPixmap &cuadro, float centro, float suelo, float alto);
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
    QVector<QPixmap> rocketDanio;
    QVector<QPixmap> rocketMuerte;
    QVector<QPixmap> soldadoMuere;
    QVector<QPixmap> jineteGalope;
    QVector<QPixmap> jineteMuere;
    QVector<QPixmap> carretaRueda;
    QVector<QPixmap> carretaHerida;
    QVector<QPixmap> carretaExplota;
    QVector<QPixmap> barrilQuieto;
    QVector<QPixmap> barrilEstalla;
    QPixmap puenteQuieto;

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
    QVector<Caida> caidas;

    QSet<int> teclas;
    float tiempo = 0.f;
    float granadaMuestra = 0.f;
    float punteroX = 0.f;
    float punteroY = 0.f;
    bool hayApunte = false;
};

#endif
