#ifndef ENTIDAD_H
#define ENTIDAD_H

#include <vector>

class ModeloFisico;

// Dice qué clase concreta es una entidad sin usar dynamic_cast.
// La capa gráfica lo usa para escoger el sprite; la lógica, para resolver choques.
enum ClaseEntidad : short { CLASE_ROCKET = 0, CLASE_ENEMIGO, CLASE_CAPITAN, CLASE_PROYECTIL, CLASE_ESTRUCTURA };

// Cualquier cosa que vive en el mapa: Rocket, enemigos, proyectiles, estructuras.
// (x, y) es la esquina superior izquierda, igual que en Qt.
// La entidad es dueña de sus modelos físicos: los borra en el destructor.

class Entidad {
protected:
    float x;
    float y;
    float ancho;
    float alto;
    bool viva;
    bool seguirModelos; //true: los modelos mueven a la entidad. false: solo se consultan (cono de la torre)
    std::vector<ModeloFisico *> modelos;

public:
    //Lanza std::invalid_argument si ancho o alto no son positivos
    Entidad(float x, float y, float ancho, float alto);
    virtual ~Entidad();

    virtual void actualizar(float dt);

    //Lo único que la capa gráfica necesita, además de posición y tamaño
    virtual short clase() const = 0;
    virtual short estadoVisual() const;

    //Reglas del juego que el Nivel aplica sin saber qué clase concreta tiene delante
    virtual bool esSolida() const;             //¿detiene las balas de Rocket?
    virtual void alChocar(Entidad &otra);      //el Nivel avisa a las dos entidades que se tocan
    virtual void recibirDanio(float cantidad);

    bool colisiona(const Entidad *otra) const;
    void agregarModelo(ModeloFisico *modelo);  //la entidad queda como dueña del modelo

    float getX() const;
    float getY() const;
    float getAncho() const;
    float getAlto() const;
    float getCentroX() const;
    float getCentroY() const;
    bool estaViva() const;
    void setPosicion(float nx, float ny);
    void matar();

    Entidad(const Entidad &) = delete;
    Entidad &operator=(const Entidad &) = delete;
};

#endif
