#ifndef ESCENA_MISION_H
#define ESCENA_MISION_H

#include <QGraphicsScene>
#include <QString>

class Sonido;

class EscenaMision : public QGraphicsScene {
public:
    using QGraphicsScene::QGraphicsScene;
    ~EscenaMision() override = default;

    void fijarSonido(Sonido *s) { sonido = s; }

    virtual void pulsar(int tecla) = 0;
    virtual void soltar(int tecla) = 0;
    virtual void apuntar(float x, float y) = 0;
    virtual void avanzar(float dt) = 0;
    virtual void fijarProporcion(float) {}

    virtual QRectF rectCamara() const = 0;
    virtual QString textoHud() const = 0;
    virtual QString mensajeFinal() const = 0;
    virtual bool gano() const = 0;
    virtual bool acabo() const = 0;

protected:
    Sonido *sonido = nullptr;
};

#endif
