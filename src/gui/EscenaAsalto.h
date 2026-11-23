#ifndef ESCENA_ASALTO_H
#define ESCENA_ASALTO_H

#include "EscenaMision.h"
#include "Marcador.h"

#include <QBrush>
#include <QHash>
#include <QPen>
#include <QPixmap>
#include <QSet>
#include <QVector>

#include <vector>

class QGraphicsEllipseItem;
class QGraphicsItem;
class QGraphicsPathItem;
class QGraphicsPixmapItem;
class QGraphicsPolygonItem;
class QGraphicsRectItem;
class QGraphicsSimpleTextItem;
class Enemigo;
class Entidad;
class Estructura;
class Juego;
class NivelAsalto;
class Rocket;

class EscenaAsalto : public EscenaMision {
public:
    EscenaAsalto();
    ~EscenaAsalto() override;

    void pulsar(int tecla) override;
    void soltar(int tecla) override;
    void apuntar(float x, float y) override;
    void avanzar(float dt) override;
    void fijarProporcion(float aspecto) override;

    QRectF rectCamara() const override;
    QString textoHud() const override;
    QString mensajeFinal() const override;
    bool gano() const override;
    bool acabo() const override;

private:
    struct Pieza {
        QGraphicsPixmapItem *item = nullptr;
        QGraphicsPixmapItem *sombra = nullptr;
        QGraphicsPixmapItem *humo = nullptr;
        QGraphicsEllipseItem *anillo = nullptr;
        qint64 llave = 0;
        int cuadro = 0;
        int variante = 0;
        short clase = 0;
        short estado = 0;
        bool espejo = false;
        bool bombeado = false;
        bool deRocket = false;
        float x = 0.f;
        float pieY = 0.f;
        float destinoX = 0.f;
        float destinoY = 0.f;
        float radio = 0.f;
        float disparoT = 0.f;
    };

    struct Efecto {
        QGraphicsPixmapItem *item = nullptr;
        short tipo = 0;
        float edad = 0.f;
        float x = 0.f;
        float y = 0.f;
        float radio = 0.f;
        bool espejo = false;
    };

    enum TipoEfecto : short { EF_EXPLOSION = 0, EF_CAIDA };

    void cargarSprites();
    void armarEscenario();
    void armarConos();
    void armarHud();
    void crearPieza(Entidad *entidad, const std::vector<Entidad *> &lista);
    void sincronizar(float dt);
    void vestirRocket(Rocket *rocket, Pieza &pieza);
    void vestirSoldado(Enemigo *enemigo, Pieza &pieza);
    void vestirEstructura(Estructura *estructura, Pieza &pieza);
    void vestirProyectil(Entidad *entidad, Pieza &pieza);
    void retirarPieza(Pieza &pieza);
    void poner(Pieza &pieza, const QPixmap &imagen, float x, float y);
    void lanzarEfecto(short tipo, float x, float y, float radio, bool espejo = false, float retraso = 0.f);
    void actualizarEfectos(float dt);
    void actualizarConos();
    void actualizarObjetivos();
    void actualizarHud();
    void dibujarMarcador();
    QGraphicsPixmapItem *tomarItem();
    void devolverItem(QGraphicsPixmapItem *item);
    QGraphicsSimpleTextItem *crearTexto(const QString &texto, int tamano, const QColor &color, QGraphicsItem *padre);
    QGraphicsPathItem *crearPildora(float ancho, float alto, const QColor &color, QGraphicsItem *padre);
    QGraphicsPathItem *crearAviso(const QString &texto, const QColor &color);
    const QPixmap &celdaRocket(int fila, int col, bool espejo) const;
    const QPixmap &celdaSoldado(int fila, int col, bool espejo) const;

    Juego *juego = nullptr;
    NivelAsalto *nivel = nullptr;

    QHash<Entidad *, Pieza> piezas;
    std::vector<Efecto> efectos;
    std::vector<QGraphicsPixmapItem *> reserva;
    std::vector<short> eventos;

    QVector<QPixmap> rocketNormal;
    QVector<QPixmap> rocketEspejo;
    QVector<QPixmap> soldadoNormal;
    QVector<QPixmap> soldadoEspejo;
    QPixmap aspectoRocket;
    QPixmap aspectoSoldado;
    float rocketPieX = 0.f;
    float rocketPieY = 0.f;
    float rocketAncho = 0.f;
    float soldadoPieX = 0.f;
    float soldadoPieY = 0.f;
    float soldadoAncho = 0.f;

    QPixmap torreA;
    QPixmap torreB;
    QPixmap polvorin;
    QPixmap polvorinRoto;
    QPixmap bandera;
    QPixmap banderaCaida;
    QPixmap canon;
    QPixmap canonEspejo;
    QPixmap canonRoto;
    QPixmap canonRotoEspejo;
    QPixmap barril;
    QPixmap barrera;
    QPixmap humoDestruccion;
    QPixmap explosion1;
    QPixmap explosion2;
    QPixmap explosion3;
    QPixmap humoSubida;
    QPixmap chispa;
    QPixmap sombraSuave;
    QPixmap balaRocket;
    QPixmap balaEnemiga;
    QPixmap bolaCanon;
    QPixmap granadaVuelo;

    PiezasHud piezasHud;
    QGraphicsPixmapItem *marcador = nullptr;

    QGraphicsRectItem *panel = nullptr;
    QGraphicsSimpleTextItem *hudReloj = nullptr;
    QGraphicsSimpleTextItem *hudEtiquetaTiempo = nullptr;
    QGraphicsSimpleTextItem *hudTarea[3] = {nullptr, nullptr, nullptr};
    QGraphicsEllipseItem *hudPunto[3] = {nullptr, nullptr, nullptr};
    QGraphicsEllipseItem *hudAlarma[3] = {nullptr, nullptr, nullptr};
    QGraphicsPathItem *avisoRefuerzos = nullptr;
    QGraphicsPathItem *avisoResistencia = nullptr;

    QVector<QGraphicsPolygonItem *> conos;
    QVector<Estructura *> torresVigiladas;
    QVector<int> conoTono;
    QBrush brochaCono[2];
    QPen lapizCono[2];
    QVector<QGraphicsPolygonItem *> flechas;
    QVector<Estructura *> objetivos;

    QSet<int> teclas;
    int cuadroActual = 0;
    int explosionesCuadro = 0;
    float tiempo = 0.f;
    float vistaAncho = 1280.f;
    float vistaAlto = 720.f;
    float camX = 0.f;
    float camY = 0.f;
    float punteroX = 0.f;
    float punteroY = 0.f;
    bool hayApunte = false;

    int hudSegundos = -1;
    int hudEstadoReloj = -1;
    int hudMarcaTareas = -1;
    int hudNivelAlarma = -1;
    bool avisoRefuerzosVisible = false;
    bool avisoResistenciaVisible = false;
    bool hudEnResistencia = false;
    short hudVidas = -1;
    int hudSalud = -1;
    short hudMunicion = -1;
    short hudGranadas = -1;
    int hudFuria = -1;

    float rocketDisparoT = 0.f;
    float rocketLanzaT = 0.f;
    float rocketDanioT = 0.f;
    float rocketMuerteT = 0.f;
    float rocketSaludPrev = -1.f;
    short rocketVidasPrev = -1;
    bool rocketMiraDer = true;
};

#endif
