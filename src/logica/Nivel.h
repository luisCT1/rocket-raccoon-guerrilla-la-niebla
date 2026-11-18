#ifndef NIVEL_H
#define NIVEL_H

#include <vector>

class Entidad;
class Rocket;

// Contrato de nivel. Emboscada, Asalto y Peñón heredan de aquí.
//
// Memoria: el Nivel es dueño de todas sus entidades y las borra en el destructor.
//
// Cada cuadro, actualizar(dt) hace en este orden:
//   1. borra las entidades que murieron en el cuadro ANTERIOR
//   2. actualiza las vivas y deja disparar a los enemigos
//   3. apoya a Rocket en el piso (vista lateral) o lo frena contra las estructuras (vista cenital)
//   4. resuelve choques y explosiones
// Una entidad que muere sigue existiendo hasta el siguiente actualizar(): la capa gráfica
// alcanza a quitar su dibujo y nunca se queda con un puntero a algo ya borrado.

class Nivel {
protected:
    float tiempo;
    bool terminado;
    bool ganado;
    std::vector<Entidad *> entidades;
    std::vector<short> eventos;
    Rocket *rocket; //acceso rápido. El dueño es 'entidades'

    float anchoMundo;
    float altoMundo;
    bool cenital;   //true: vista desde arriba. false: vista lateral con gravedad
    float ySuelo;   //vista lateral: altura del piso
    float huecoIni; //vista lateral: tramo sin piso (barranco). Iguales = no hay
    float huecoFin;
    float camaraX;
    std::vector<Entidad *> plataformas; //donde Rocket se puede parar. Sin dueño
    Entidad *apoyo;                     //plataforma sobre la que está parado, o nullptr
    float seguroX;  //último punto firme: ahí reaparece si cae al barranco
    float seguroY;

    void emitirEvento(short codigo);
    void terminar(bool gano); //marca el final y emite EV_VICTORIA o EV_DERROTA
    void registrarPlataforma(Entidad *p);
    //Daña a lo que esté dentro del radio. deRocket: daña enemigos y estructuras; si no, daña a Rocket.
    void explosionEn(float cx, float cy, float radio, float danio, bool deRocket);
    void notificarGolpe();
    short contarVivos(short clase) const;
    //Se llama justo antes de borrar una entidad. La subclase que guarde un puntero a ella lo suelta aquí
    //(y llama a Nivel::alBorrar). Así ningún nivel se queda con un puntero colgando.
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

    //La subclase llama primero a Nivel::actualizar(dt) y después aplica sus reglas
    virtual void actualizar(float dt);
    virtual bool victoria() const = 0;
    virtual bool derrota() const = 0;

    void agregarEntidad(Entidad *e); //el Nivel queda como dueño
    void colisiones();
    void eliminarMuertas();
    Rocket *jugador() const;

    //Órdenes del jugador. La capa gráfica solo traduce teclas y ratón a estas llamadas.
    void jugadorMueve(char dir); //'L', 'R', 'U' o 'D', en cada cuadro mientras la tecla esté abajo
    void jugadorSalta();
    void jugadorDispara(float objX, float objY); //punto del mundo al que apunta
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
