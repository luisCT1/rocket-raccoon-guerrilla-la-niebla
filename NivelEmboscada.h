#ifndef NIVEL_EMBOSCADA_H
#define NIVEL_EMBOSCADA_H

#include "Nivel.h"

class Estructura;
class Enemigo;

class NivelEmboscada : public Nivel {
private:
    float posCamara;
    short partesRestantes;
    bool correoEscapo;
    bool correoDetenido;
    float intervaloOleada;
    float tiempoOleada;

    Estructura *puente;
    Estructura *carretas[3];
    Estructura *barriles[3];
    Enemigo *correo;

    void soltarCorreo();
    void revisarCorreo();

protected:
    void alBorrar(Entidad *e) override;

public:
    NivelEmboscada();
    ~NivelEmboscada() override;

    void actualizar(float dt) override;
    bool victoria() const override;
    bool derrota() const override;

    void generarOleada();
    void moverCamara(float dt);

    short getPartesRestantes() const;
    bool correoEnFuga() const;
    bool getCorreoEscapo() const;
    float getHuecoInicio() const;
    float getHuecoFin() const;
    float getFinCamino() const;
    Estructura *getPuente() const;
};

#endif
