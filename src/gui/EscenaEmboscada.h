#ifndef ESCENA_EMBOSCADA_H
#define ESCENA_EMBOSCADA_H

#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QSet>
#include <QVector>

//Nivel 1 en pantalla: fondo fijo, puente que oscila y el mapache con sus animaciones.

class EscenaEmboscada : public QGraphicsScene {
public:
    EscenaEmboscada();

    void pulsar(int tecla);
    void soltar(int tecla);
    void avanzar(float dt);

private:
    enum class Pose {
        Quieto,
        Camina,
        Salta,
        Dispara,
        Impulso,
        Furia,
        Herido,
        Caido
    };

    struct Bala {
        QGraphicsEllipseItem *dibujo = nullptr;
        float x = 0.f;
        float y = 0.f;
        float vx = 0.f;
        float vy = 0.f;
        float x0 = 0.f;
        float y0 = 0.f;
        float edad = 0.f;
        bool delJugador = true;
        bool granada = false;
        int dano = 1;
    };

    struct Destello {
        QGraphicsEllipseItem *dibujo = nullptr;
        float vida = 0.2f;
    };

    struct Actor {
        QGraphicsPixmapItem *dibujo = nullptr;
        float x = 0.f;
        float y = 0.f;
        float vel = -40.f;
        float anim = 0.f;
        float poseT = 0.f;
        float recarga = 0.8f;
        int vida = 2;
        int vidaMax = 2;
        int mira = -1;
        int tipo = 0;
        float alto = 130.f;
        float ancho = 50.f;
        bool vivo = true;
    };

    void cargarSprites();
    void ponerMapache();
    void ponerPuente(float osc);
    float sueloEn(float x, float osc) const;
    const QVector<QPixmap> &hojaDe(Pose pose) const;
    const QVector<int> &piesDe(Pose pose) const;
    int cuadroPuente() const;
    float origenXPuente(int cuadro) const;
    float origenYPuente(int cuadro, float osc) const;
    void prepararConvoy();
    void ponerActor(Actor &actor);
    void avanzarConvoy(float dt, float osc);
    void dispararFusil();
    void lanzarGranada();
    void explotar(float gx, float gy);
    void herirJugador();
    bool golpeaActor(const Actor &actor, float bx, float by) const;
    bool golpeaJugador(float bx, float by) const;

    QGraphicsPixmapItem *fondo = nullptr;
    QGraphicsPixmapItem *puente = nullptr;
    QGraphicsPixmapItem *mapache = nullptr;

    struct MarcoPuente {
        QPixmap pix;
        QVector<int> cubierta;
        float medio = 0.f;
        float ancla = 0.f;
    };

    QVector<QPixmap> caminar;
    QVector<QPixmap> saltar;
    QVector<QPixmap> disparar;
    QVector<QPixmap> impulso;
    QVector<QPixmap> furia;
    QVector<QPixmap> herido;
    QVector<QPixmap> caido;
    QVector<int> piesCaminar;
    QVector<int> piesSaltar;
    QVector<int> piesDisparar;
    QVector<int> piesImpulso;
    QVector<int> piesFuria;
    QVector<int> piesHerido;
    QVector<int> piesCaido;
    QVector<MarcoPuente> marcosPuente;
    float escalaPuente = 1.f;

    QVector<QPixmap> soldadoCamina;
    QVector<QPixmap> soldadoDispara;
    QVector<QPixmap> soldadoMuere;
    QVector<int> piesSoldadoCamina;
    QVector<int> piesSoldadoDispara;
    QVector<int> piesSoldadoMuere;
    QVector<QPixmap> jineteGalope;
    QVector<QPixmap> jineteMuere;
    QVector<int> piesJineteGalope;
    QVector<int> piesJineteMuere;
    QVector<QPixmap> carretaRueda;
    QVector<QPixmap> carretaHerida;
    QVector<QPixmap> carretaExplota;
    QVector<int> piesCarretaRueda;
    QVector<int> piesCarretaHerida;
    QVector<int> piesCarretaExplota;

    QSet<int> teclas;
    QVector<Bala> balas;
    QVector<Actor> convoy;
    QVector<Destello> destellos;

    Pose pose = Pose::Quieto;
    float x = 300.f;
    float y = 620.f;
    float vy = 0.f;
    float tiempo = 0.f;
    float anim = 0.f;
    float poseTimer = 0.f;
    float furiaTimer = 0.f;
    float recargaDisparo = 0.f;
    float invulnerable = 0.f;
    int mira = 1;
    int vidas = 3;
    int granadas = 5;
    bool enAire = false;
    bool vivo = true;
};
#endif
