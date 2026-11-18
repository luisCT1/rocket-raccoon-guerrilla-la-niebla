#ifndef NIVEL_EMBOSCADA_H
#define NIVEL_EMBOSCADA_H

#include "Nivel.h"

class Estructura;
class Enemigo;

// Nivel 1. Emboscada al convoy. Vista lateral con scroll.
// Rocket cruza el barranco por el puente colgante, que sube y baja, y destroza las tres carretas.
// Al caer la última, el correo sale corriendo a avisar: si llega al final del camino, la misión falla.
// Los barriles de pólvora explotan y se llevan la carreta que tengan al lado.

class NivelEmboscada : public Nivel {
private:
    float posCamara;
    short partesRestantes;
    bool correoEscapo;
    bool correoDetenido;
    float intervaloOleada;
    float tiempoOleada;

    //Sin dueño: las estructuras no se borran hasta que termina el nivel
    Estructura *puente;
    Estructura *carretas[3];
    Estructura *barriles[3];
    Enemigo *correo; //nullptr hasta que sale, y otra vez nullptr en cuanto cae

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

    //Lectura para la capa gráfica
    short getPartesRestantes() const;
    bool correoEnFuga() const;
    bool getCorreoEscapo() const;
    float getHuecoInicio() const;
    float getHuecoFin() const;
    float getFinCamino() const;
    Estructura *getPuente() const;
};

#endif
