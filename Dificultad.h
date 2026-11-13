#ifndef DIFICULTAD_H
#define DIFICULTAD_H

class Capitan;

class Dificultad {
public:
    explicit Dificultad(short nivel);
    void fijar(short nivel);
    void aplicar(Capitan *capitan) const;

    short getNivel() const { return nivel; }
    float getRetrasoPercepcion() const { return retrasoPercepcion; }
    float getTasaAprendizaje() const { return tasaAprendizaje; }
    float getAgresividad() const { return agresividad; }
    float getVelocidadAtaques() const { return velocidadAtaques; }

private:
    short nivel;
    float retrasoPercepcion;
    float tasaAprendizaje;
    float agresividad;
    float velocidadAtaques;
};

#endif
