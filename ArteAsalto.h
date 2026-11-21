#ifndef ARTEASALTO_H
#define ARTEASALTO_H

#include <QHash>
#include <QPixmap>
#include <QVector>

#include <vector>

class QPainter;
class Enemigo;
class Entidad;
class Estructura;
class NivelAsalto;
class Rocket;
class Sprites;

class ArteAsalto {
private:
    struct Rastro {
        short clase = 0;
        int cuadro = 0;
        int variante = 0;
        bool espejo = false;
        bool seMueve = false;
        bool bombeado = false;
        bool deRocket = false;
        bool rota = false;
        float x = 0.f;
        float pieY = 0.f;
        float destinoX = 0.f;
        float destinoY = 0.f;
        float radio = 0.f;
        float disparoT = 0.f;
    };

    struct Efecto {
        short tipo = 0;
        float edad = 0.f;
        float x = 0.f;
        float y = 0.f;
        float radio = 0.f;
        bool espejo = false;
    };

    struct Orden {
        float y;
        const Entidad *entidad;
    };

    enum TipoEfecto : short { EF_EXPLOSION = 0, EF_CHISPA, EF_CAIDA };

    QHash<const Entidad *, Rastro> rastros;
    QVector<Efecto> efectos;
    std::vector<Orden> orden;

    QPixmap fondo;
    QVector<QPixmap> rocketNormal;
    QVector<QPixmap> rocketEspejo;
    QVector<QPixmap> soldadoNormal;
    QVector<QPixmap> soldadoEspejo;
    QPixmap aspectoRocket;
    QPixmap aspectoSoldado;
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

    float rocketPieX;
    float rocketPieY;
    float rocketAncho;
    float soldadoPieX;
    float soldadoPieY;
    float soldadoAncho;

    float tiempo;
    int cuadroActual;
    float punteroX;
    float punteroY;
    float rocketDisparoT;
    float rocketLanzaT;
    float rocketDanioT;
    float rocketMuerteT;
    float rocketSaludPrev;
    short rocketVidasPrev;
    bool rocketMiraDer;
    bool hayPuntero;
    bool fondoAjustado;

    const QPixmap &celdaRocket(int fila, int col, bool espejo) const;
    const QPixmap &celdaSoldado(int fila, int col, bool espejo) const;
    void cargar(Sprites *sprites);
    void crearRastro(const Entidad *entidad, NivelAsalto *nivel);
    void lanzarEfecto(short tipo, float x, float y, float radio, bool espejo = false);
    void retirarRastro(const Rastro &rastro);
    void avanzarEfectos(float dt);
    void seguirEstructura(const Estructura *estructura, Rastro &rastro);
    void avanzarEntidades(NivelAsalto *nivel, float dt);

    void dibujarFondo(QPainter &p, const NivelAsalto *nivel) const;
    void dibujarSalida(QPainter &p, const NivelAsalto *nivel) const;
    void dibujarConos(QPainter &p, const NivelAsalto *nivel) const;
    void dibujarAvisos(QPainter &p) const;
    void dibujarSombra(QPainter &p, float cx, float pie) const;
    void dibujarRocket(QPainter &p, const Rocket *rocket) const;
    void dibujarSoldado(QPainter &p, const Enemigo *enemigo) const;
    const QPixmap &imagenEstructura(const NivelAsalto *nivel, const Estructura *estructura, float &desfase) const;
    void dibujarEstructura(QPainter &p, const NivelAsalto *nivel, const Estructura *estructura) const;
    void dibujarHumo(QPainter &p, const NivelAsalto *nivel, const Estructura *estructura) const;
    void dibujarProyectiles(QPainter &p, const NivelAsalto *nivel) const;
    void dibujarEfectos(QPainter &p, bool cuerpos) const;
    void dibujarFlechas(QPainter &p, const NivelAsalto *nivel) const;

public:
    explicit ArteAsalto(Sprites *sprites);

    void reiniciar();
    void actualizar(NivelAsalto *nivel, float dt, float mundoX, float mundoY);
    void dibujar(QPainter &p, NivelAsalto *nivel);
};

#endif
