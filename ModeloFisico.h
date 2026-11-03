#ifndef MODELOFISICO_H
#define MODELOFISICO_H

constexpr float PI_F     = 3.14159265358979f;
constexpr float DOS_PI_F = 6.28318530717959f;

class ModeloFisico {
public:
    ModeloFisico() : t(0.0f), activo(true) {}
    virtual ~ModeloFisico() = default;

    virtual void avanzar(float dt) = 0;
    virtual void posicion(float &x, float &y) const = 0;
    virtual void reiniciar() { t = 0.0f; }

    void activar(bool a) { activo = a; }
    bool estaActivo() const { return activo; }

protected:
    float t;
    bool  activo;
};

#endif // MODELOFISICO_H
