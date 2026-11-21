#ifndef RECOGIBLE_H
#define RECOGIBLE_H

#include "Entidad.h"

enum TipoRecogible : short { RECO_VIDA = 0, RECO_MUNICION, RECO_GRANADA, RECO_FURIA };

class Recogible : public Entidad {
public:
    Recogible(short tipo_, float x_, float y_);

    short clase() const override;
    bool esSolida() const override;
    void actualizar(float dt) override;
    void alChocar(Entidad &otra) override;

    short getTipo() const;
    bool estaDisponible() const;

private:
    short tipo;
    float espera;
};

#endif
