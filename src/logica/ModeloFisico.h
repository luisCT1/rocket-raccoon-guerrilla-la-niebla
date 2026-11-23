#ifndef MODELO_FISICO_H
#define MODELO_FISICO_H


constexpr float PI_F = 3.14159265358979f;
constexpr float DOS_PI_F = 6.28318530717959f;
constexpr float GRAVEDAD = 900.0f;


class ModeloFisico {
protected:
    float t;
    bool activo;

public:
    ModeloFisico();
    virtual ~ModeloFisico();

    virtual void avanzar(float dt) = 0;
    virtual void posicion(float &outX, float &outY) const = 0;
    virtual void reiniciar();

    void activar(bool a);
    bool estaActivo() const;
    float tiempo() const;
};

#endif
