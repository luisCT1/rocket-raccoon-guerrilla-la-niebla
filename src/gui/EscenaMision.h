#ifndef ESCENA_MISION_H
#define ESCENA_MISION_H

#include <QGraphicsScene>
#include <QString>

class EscenaMision : public QGraphicsScene {
public:
    using QGraphicsScene::QGraphicsScene;
    ~EscenaMision() override = default;

    virtual void pulsar(int tecla) = 0;
    virtual void soltar(int tecla) = 0;
    virtual void mantener(int tecla) = 0;
    virtual void apuntar(float x, float y) = 0;
    virtual void avanzar(float dt) = 0;

    virtual QRectF rectCamara() const = 0;
    virtual QString textoHud() const = 0;
    virtual QString mensajeFinal() const = 0;
    virtual bool gano() const = 0;
    virtual bool acabo() const = 0;
    virtual bool enFuria() const { return false; }
};

#endif
