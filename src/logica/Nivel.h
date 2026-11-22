#ifndef NIVEL_H
#define NIVEL_H

#include <vector>

class Entidad;
class Rocket;

class Nivel {
protected:
    float tiempo;
    bool terminado;
    bool ganado;
    std::vector<Entidad *> entidades;
    std::vector<short> eventos;
    Rocket *rocket;

    float anchoMundo;
    float altoMundo;
    bool cenital;
    float ySuelo;
    float huecoIni;
    float huecoFin;
    float camaraX;
    std::vector<Entidad *> plataformas;
    Entidad *apoyo;
    float seguroX;
    float seguroY;

    void emitirEvento(short codigo);
    void terminar(bool gano);
    void registrarPlataforma(Entidad *p);

    void explosionEn(float cx, float cy, float radio, float danio, bool deRocket);
    void notificarGolpe();
    short contarVivos(short clase) const;

    virtual void alBorrar(Entidad *e);

private:
    void disparosEnemigos();
    void resolverApoyo();
    void bloquearContraEstructuras();
    void limitarAlMundo();
    void explosiones();

public:
    Nivel();
    virtual ~Nivel();

    virtual void actualizar(float dt);
    virtual bool victoria() const = 0;
    virtual bool derrota() const = 0;

    void agregarEntidad(Entidad *e); //el Nivel queda como dueño
    void colisiones();
    void eliminarMuertas();
    Rocket *jugador() const;

    void jugadorMueve(char dir);
    void jugadorSalta();
    void jugadorDispara(float objX, float objY);
    void jugadorLanzaGranada(float objX, float objY);
    void jugadorDash();
    void jugadorActivaFuria();

    //Lectura para la capa gráfica
    const std::vector<Entidad *> &obtenerEntidades() const;
    void tomarEventos(std::vector<short> &destino); //intercambia los vectores: no reserva memoria
    float tiempoTranscurrido() const;
    bool estaTerminado() const;
    bool fueGanado() const;
    bool esCenital() const;
    float getAnchoMundo() const;
    float getAltoMundo() const;
    float getCamaraX() const;
    float getSuelo() const;

    Nivel(const Nivel &) = delete;
    Nivel &operator=(const Nivel &) = delete;
};

#endif
