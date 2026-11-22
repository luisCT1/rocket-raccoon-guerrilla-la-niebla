#include "Rocket.h"
#include "Proyectil.h"
#include "MovParabolico.h"
#include <cmath>
#include <algorithm>

namespace {
constexpr float V_CAMINAR = 180.f;
constexpr float V_FURIA = 220.f;
constexpr float G_SALTO = 1600.f;        //gravedad del salto (px/s^2)
constexpr float V_SALTO = 620.f;         //sube unos 120 px
constexpr float V_CAIDA_MAX = 900.f;
constexpr float V_DISPARO = 520.f;
constexpr float V_GRANADA = 380.f;
constexpr float ALCANCE_GRANADA = 450.f;
constexpr float CAIDA_BALA = 0.08f;      //fracción de GRAVEDAD que curva la bala en vista lateral
constexpr float DANIO_BALA = 25.f;
constexpr float DANIO_GRANADA = 60.f;
constexpr float RADIO_BALA = 12.f;
constexpr float RADIO_GRANADA = 70.f;
constexpr float CADENCIA = 0.30f;
constexpr float CADENCIA_FURIA = 0.18f;
constexpr float DISPERSION_FURIA = 0.16f; //radianes
constexpr float DURACION_FURIA = 5.f;
constexpr float V_DASH = 600.f;
constexpr float DURACION_DASH = 0.2f;    //600 px/s durante 0.2 s = 120 px
constexpr float COOLDOWN_DASH = 1.2f;
constexpr float BLOQUEO_POST_DASH = 0.25f;
constexpr float RUIDO_DASH = 200.f;      //el jefe oye el dash a esta distancia
constexpr float CAIDA_RUIDO = 180.f;
constexpr float TIEMPO_INVULNERABLE = 1.f;
constexpr float AVISO_GRANADA = 0.5f;
constexpr short MUNICION_MAX = 30;
constexpr float RECARGA_BALA_FURIA = 0.12f;
constexpr float RECARGA_BALA = 1.2f;     //recupera una bala cada tanto: nunca queda sin poder disparar
constexpr float DIAGONAL = 0.7071f;
}

Rocket::Rocket(float x_, float y_)
    : Entidad(x_, y_, 48.f, 64.f),
      vidas(3),
      salud(100.f),
      saludMax(100.f),
      cayo(false),
      municion(MUNICION_MAX),
      granadas(5),
      barraFuria(0.f),
      furioso(false),
      tiempoFuria(0.f),
      recargaDash(0.f),
      bloqueoDisparo(0.f),
      radioRuido(0.f),
      direccion('R'),
      enSuelo(true),
      cenital(false),
      intencionX(0.f),
      intencionY(0.f),
      moviendose(false),
      velY(0.f),
      xAnterior(x_),
      yAnterior(y_),
      tiempoDash(0.f),
      dashX(0.f),
      dashY(0.f),
      invulnerable(0.f),
      recargaDisparo(0.f),
      recargaMunicion(0.f),
      avisoGranada(0.f),
      golpePendiente(false) {}

Rocket::~Rocket() {}

void Rocket::actualizar(float dt) {
    xAnterior = x;
    yAnterior = y;
    Entidad::actualizar(dt);

    if (recargaDash > 0.f) recargaDash -= dt;
    if (bloqueoDisparo > 0.f) bloqueoDisparo -= dt;
    if (recargaDisparo > 0.f) recargaDisparo -= dt;
    if (invulnerable > 0.f) invulnerable -= dt;
    if (avisoGranada > 0.f) avisoGranada -= dt;
    if (radioRuido > 0.f) radioRuido = std::max(0.f, radioRuido - CAIDA_RUIDO * dt);

    if (furioso) {
        tiempoFuria -= dt;
        if (tiempoFuria <= 0.f) {
            furioso = false;
            tiempoFuria = 0.f;
        }
    }

    if (municion < MUNICION_MAX) {
        recargaMunicion += dt;
        const float periodo = furioso ? RECARGA_BALA_FURIA : RECARGA_BALA;
        if (recargaMunicion >= periodo) {
            recargaMunicion -= periodo;
            ++municion;
        }
    }

    //Movimiento: todo va multiplicado por dt, así no depende de los cuadros por segundo
    const float ix = intencionX;
    const float iy = cenital ? intencionY : 0.f;
    if (tiempoDash > 0.f) {
        //Solo se mueve lo que queda de dash: recorre 120 px sin importar los cuadros por segundo
        const float paso = std::min(dt, tiempoDash);
        x += dashX * V_DASH * paso;
        y += dashY * V_DASH * paso;
        tiempoDash -= dt;
    } else {
        float vel = furioso ? V_FURIA : V_CAMINAR;
        if (ix != 0.f && iy != 0.f) vel *= DIAGONAL; //en diagonal no corre más rápido
        x += ix * vel * dt;
        y += iy * vel * dt;
    }
    moviendose = ix != 0.f || iy != 0.f;

    //Vista lateral: en el aire lo gobierna la gravedad. El Nivel decide cuándo aterriza.
    if (!cenital && !enSuelo) {
        velY = std::min(velY + G_SALTO * dt, V_CAIDA_MAX);
        y += velY * dt;
    }

    intencionX = 0.f;
    intencionY = 0.f;
}

short Rocket::clase() const {
    return CLASE_ROCKET;
}

short Rocket::estadoVisual() const {
    if (tiempoDash > 0.f) return 3;
    if (!cenital && !enSuelo) return 2;
    return moviendose ? 1 : 0;
}

void Rocket::mover(char dir) {
    switch (dir) {
    case 'L': case 'l': intencionX = -1.f; direccion = 'L'; break;
    case 'R': case 'r': intencionX = 1.f; direccion = 'R'; break;
    case 'U': case 'u': if (cenital) { intencionY = -1.f; direccion = 'U'; } break;
    case 'D': case 'd': if (cenital) { intencionY = 1.f; direccion = 'D'; } break;
    default: break;
    }
}

bool Rocket::saltar() {
    if (cenital || !enSuelo) return false;
    enSuelo = false;
    velY = -V_SALTO;
    return true;
}

Proyectil *Rocket::disparar(float objX, float objY) {
    if (bloqueoDisparo > 0.f || recargaDisparo > 0.f) return nullptr;
    const short costo = furioso ? 2 : 1; //en furia gasta el doble
    if (municion < costo) return nullptr;
    municion = static_cast<short>(municion - costo);
    recargaDisparo = furioso ? CADENCIA_FURIA : CADENCIA;

    const float cx = x + ancho * 0.5f;
    const float cy = y + alto * 0.35f;
    float ang = std::atan2(-(objY - cy), objX - cx); //positivo hacia arriba
    if (furioso) {
        //En furia dispara más chueco a propósito
        ang += static_cast<float>((municion % 3) - 1) * DISPERSION_FURIA;
    }
    if (!cenital) direccion = objX >= cx ? 'R' : 'L'; //de lado, mira hacia donde dispara

    Proyectil *p = new Proyectil(cx - 8.f, cy - 8.f, RADIO_BALA, true);
    p->fijarDanio(furioso ? DANIO_BALA * 1.5f : DANIO_BALA);
    p->fijarTiempoVida(1.6f);
    p->agregarModelo(new MovParabolico(p->getX(), p->getY(), V_DISPARO, ang, cenital ? 0.f : GRAVEDAD * CAIDA_BALA));
    return p;
}

Proyectil *Rocket::lanzarGranada(float objX, float objY) {
    if (granadas <= 0 || bloqueoDisparo > 0.f) return nullptr;
    --granadas;
    avisoGranada = AVISO_GRANADA;

    const float cx = x + ancho * 0.5f;
    const float cy = y + alto * 0.2f;

    //La granada cae donde se apunta, pero no más lejos que su alcance
    float dx = objX - cx;
    float dy = objY - cy;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist > ALCANCE_GRANADA) {
        dx *= ALCANCE_GRANADA / dist;
        dy *= ALCANCE_GRANADA / dist;
        dist = ALCANCE_GRANADA;
    }
    const float vuelo = std::min(std::max(dist / V_GRANADA, 0.5f), 1.4f);

    Proyectil *p = new Proyectil(cx - 8.f, cy - 8.f, RADIO_GRANADA, true);
    p->fijarDanio(DANIO_GRANADA);
    p->fijarBombeado(true);
    p->fijarTiempoVida(vuelo);
    p->fijarDestino(cx + dx, cy + dy);
    p->agregarModelo(MovParabolico::haciaObjetivo(p->getX(), p->getY(), cx + dx - 8.f, cy + dy - 8.f, vuelo, GRAVEDAD));
    return p;
}

bool Rocket::dash(char dir) {
    if (recargaDash > 0.f) return false;

    dashX = 0.f;
    dashY = 0.f;
    switch (dir) {
    case 'L': case 'l': dashX = -1.f; break;
    case 'U': case 'u': if (cenital) dashY = -1.f; else dashX = direccion == 'L' ? -1.f : 1.f; break;
    case 'D': case 'd': if (cenital) dashY = 1.f; else dashX = direccion == 'L' ? -1.f : 1.f; break;
    default: dashX = 1.f; break;
    }

    tiempoDash = DURACION_DASH;
    recargaDash = COOLDOWN_DASH;
    bloqueoDisparo = DURACION_DASH + BLOQUEO_POST_DASH; //después del dash queda un momento sin disparar
    radioRuido = RUIDO_DASH; //el jefe puede oír este ruido
    return true;
}

bool Rocket::activarFuria() {
    if (furioso || barraFuria < 1.f) return false;
    furioso = true;
    tiempoFuria = DURACION_FURIA;
    barraFuria = 0.f;
    return true;
}

void Rocket::recibirDanio(float cantidad) {
    //Durante el dash no lo tocan: por eso sirve para zafarse
    if (vidas <= 0 || invulnerable > 0.f || tiempoDash > 0.f) return;
    if (cantidad < 8.f) cantidad = 25.f; //rozarse con un soldado también duele
    cayo = false;
    salud -= cantidad;
    golpePendiente = true;
    if (salud > 0.f) {
        invulnerable = 0.4f;
        return;
    }
    salud = 0.f;
    --vidas;
    if (vidas > 0) {
        salud = saludMax;
        invulnerable = TIEMPO_INVULNERABLE;
    }
}

bool Rocket::recibirDanio() {
    const float antes = salud;
    const short oportunidades = vidas;
    recibirDanio(34.f);
    return salud < antes || vidas < oportunidades;
}

void Rocket::perderVida() {
    if (vidas <= 0) return;
    --vidas;
    cayo = true;
    golpePendiente = true;
    salud = vidas > 0 ? saludMax : 0.f;
}

void Rocket::sumarFuria(float cantidad) {
    if (furioso) return;
    barraFuria = std::min(1.f, std::max(0.f, barraFuria + cantidad));
}

float Rocket::ruidoActual() const {
    return radioRuido;
}

bool Rocket::lanzoGranadaReciente() const {
    return avisoGranada > 0.f;
}

bool Rocket::consumirGolpe() {
    const bool r = golpePendiente;
    golpePendiente = false;
    return r;
}

void Rocket::fijarCenital(bool c) {
    cenital = c;
}

void Rocket::aterrizarEn(float ySuperficie) {
    y = ySuperficie - alto;
    velY = 0.f;
    enSuelo = true;
}

void Rocket::soltarSuelo() {
    enSuelo = false;
}

void Rocket::reaparecerEn(float nx, float ny) {
    x = nx;
    y = ny;
    xAnterior = nx;
    yAnterior = ny;
    velY = 0.f;
    enSuelo = false;
    tiempoDash = 0.f;
    invulnerable = TIEMPO_INVULNERABLE * 1.5f;
}

short Rocket::getVidas() const { return vidas; }
float Rocket::getSalud() const { return salud; }
float Rocket::getSaludMax() const { return saludMax; }
bool Rocket::cayoAlVacio() const { return cayo; }
short Rocket::getMunicion() const { return municion; }
short Rocket::getGranadas() const { return granadas; }
float Rocket::getBarraFuria() const { return barraFuria; }
float Rocket::getVelY() const { return velY; }
float Rocket::getXAnterior() const { return xAnterior; }
float Rocket::getYAnterior() const { return yAnterior; }
char Rocket::getDireccion() const { return direccion; }
bool Rocket::estaFurioso() const { return furioso; }
bool Rocket::estaEnSuelo() const { return enSuelo; }
bool Rocket::esInvulnerable() const { return invulnerable > 0.f; }
bool Rocket::dashListo() const { return recargaDash <= 0.f; }
