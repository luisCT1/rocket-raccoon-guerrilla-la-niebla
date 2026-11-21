#include "Rocket.h"
#include "Proyectil.h"
#include "MovParabolico.h"
#include "Recogible.h"
#include <cmath>
#include <algorithm>

namespace {
constexpr float V_CAMINAR = 180.f;
constexpr float V_FURIA = 340.f;
constexpr float G_SALTO = 1600.f;
constexpr float V_SALTO = 620.f;
constexpr float V_CAIDA_MAX = 900.f;
constexpr float V_DISPARO = 520.f;
constexpr float V_GRANADA = 380.f;
constexpr float ALCANCE_GRANADA = 450.f;
constexpr float CAIDA_BALA = 0.08f;
constexpr float DANIO_BALA = 25.f;
constexpr float DANIO_GRANADA = 60.f;
constexpr float RADIO_BALA = 12.f;
constexpr float RADIO_GRANADA = 70.f;
constexpr float CADENCIA = 0.30f;
constexpr float CADENCIA_FURIA = 0.09f;
constexpr float DISPERSION_FURIA = 0.16f;
constexpr float DURACION_FURIA = 6.f;
constexpr float V_DASH = 600.f;
constexpr float DURACION_DASH = 0.2f;
constexpr float COOLDOWN_DASH = 1.2f;
constexpr float BLOQUEO_POST_DASH = 0.25f;
constexpr float RUIDO_DASH = 200.f;
constexpr float CAIDA_RUIDO = 180.f;
constexpr float TIEMPO_INVULNERABLE = 1.f;
constexpr float AVISO_GRANADA = 0.5f;
constexpr short MUNICION_MAX = 20;
constexpr short GRANADAS_MAX = 5;
constexpr short VIDAS_MAX = 3;
constexpr float DIAGONAL = 0.7071f;
}

Rocket::Rocket(float x_, float y_)
    : Entidad(x_, y_, 48.f, 64.f),
      vidas(VIDAS_MAX),
      salud(100.f),
      saludMax(100.f),
      cayo(false),
      municion(MUNICION_MAX),
      granadas(GRANADAS_MAX),
      barraFuria(0.f),
      furioso(false),
      tiempoFuria(0.f),
      recargaDash(0.f),
      bloqueoDisparo(0.f),
      radioRuido(0.f),
      direccion('R'),
      enSuelo(true),
      agachado(false),
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
      avisoGranada(0.f),
      golpePendiente(false),
      recogidaPendiente(false),
      tipoRecogida(-1),
      pasoPendiente(false),
      pasoAcum(0.f),
      agachadoAntes(false),
      agacharPendiente(false),
      aterrizajePendiente(false) {}

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
    if (!enSuelo || cenital) agachado = false;

    const float ix = intencionX;
    const float iy = cenital ? intencionY : 0.f;
    if (tiempoDash > 0.f) {

        const float paso = std::min(dt, tiempoDash);
        x += dashX * V_DASH * paso;
        y += dashY * V_DASH * paso;
        tiempoDash -= dt;
    } else {
        float vel = furioso ? V_FURIA : V_CAMINAR;
        if (agachado && !furioso) vel *= 0.45f;
        if (ix != 0.f && iy != 0.f) vel *= DIAGONAL;
        x += ix * vel * dt;
        y += iy * vel * dt;
    }
    moviendose = ix != 0.f || iy != 0.f;

    if (!cenital && !enSuelo) {
        velY = std::min(velY + G_SALTO * dt, V_CAIDA_MAX);
        y += velY * dt;
    }

    intencionX = 0.f;
    intencionY = 0.f;

    const bool camina = moviendose && tiempoDash <= 0.f && (cenital || enSuelo);
    if (camina) {
        pasoAcum += dt;
        float cada = furioso ? 0.18f : 0.34f;
        if (agachado && !furioso) cada = 0.52f;
        if (pasoAcum >= cada) {
            pasoAcum -= cada;
            pasoPendiente = true;
        }
    } else if (tiempoDash <= 0.f) {
        pasoAcum = 0.f;
    }

    if (agachado != agachadoAntes)
        agacharPendiente = true;
    agachadoAntes = agachado;
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
    if (cenital || !enSuelo || agachado) return false;
    enSuelo = false;
    velY = furioso ? -V_SALTO * 1.2f : -V_SALTO;
    return true;
}

Proyectil *Rocket::disparar(float objX, float objY) {
    if (bloqueoDisparo > 0.f || recargaDisparo > 0.f) return nullptr;
    const short costo = furioso ? 2 : 1;
    if (municion < costo) return nullptr;
    municion = static_cast<short>(municion - costo);
    recargaDisparo = furioso ? CADENCIA_FURIA : CADENCIA;

    const float cx = x + ancho * 0.5f;
    const float cy = y + alto * 0.35f;
    float ang = std::atan2(-(objY - cy), objX - cx);
    if (furioso) {

        ang += static_cast<float>((municion % 3) - 1) * DISPERSION_FURIA;
    }
    if (!cenital) direccion = objX >= cx ? 'R' : 'L';

    Proyectil *p = new Proyectil(cx - 8.f, cy - 8.f, RADIO_BALA, true);
    p->fijarDanio(furioso ? DANIO_BALA * 2.f : DANIO_BALA);
    p->fijarTiempoVida(1.6f);
    const float velDisparo = furioso ? V_DISPARO * 1.45f : V_DISPARO;
    p->agregarModelo(new MovParabolico(p->getX(), p->getY(), velDisparo, ang, cenital ? 0.f : GRAVEDAD * CAIDA_BALA));
    return p;
}

Proyectil *Rocket::lanzarGranada(float objX, float objY) {
    if (granadas <= 0 || bloqueoDisparo > 0.f) return nullptr;
    --granadas;
    avisoGranada = AVISO_GRANADA;

    const float cx = x + ancho * 0.5f;
    const float cy = y + alto * 0.2f;

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
    bloqueoDisparo = DURACION_DASH + BLOQUEO_POST_DASH;
    radioRuido = RUIDO_DASH;
    return true;
}

void Rocket::fijarAgachado(bool agacha) {
    agachado = agacha && enSuelo && !cenital;
}

bool Rocket::estaAgachado() const {
    return agachado;
}

bool Rocket::activarFuria() {
    if (furioso || barraFuria < 0.2f) return false;
    furioso = true;
    tiempoFuria = DURACION_FURIA;
    barraFuria = 0.f;
    return true;
}

void Rocket::recibirDanio(float cantidad) {

    if (vidas <= 0 || invulnerable > 0.f || tiempoDash > 0.f) return;
    if (cantidad < 8.f) cantidad = 25.f;
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

bool Rocket::recogerVida() {
    if (vidas >= VIDAS_MAX && salud >= saludMax)
        return false;
    if (vidas < VIDAS_MAX)
        ++vidas;
    salud = saludMax;
    cayo = false;
    recogidaPendiente = true;
    tipoRecogida = RECO_VIDA;
    return true;
}

bool Rocket::recogerMunicion(short cantidad) {
    if (cantidad <= 0 || municion >= MUNICION_MAX)
        return false;
    municion = std::min<short>(MUNICION_MAX, static_cast<short>(municion + cantidad));
    recogidaPendiente = true;
    tipoRecogida = RECO_MUNICION;
    return true;
}

bool Rocket::recogerGranadas(short cantidad) {
    if (cantidad <= 0 || granadas >= GRANADAS_MAX)
        return false;
    granadas = std::min<short>(GRANADAS_MAX, static_cast<short>(granadas + cantidad));
    recogidaPendiente = true;
    tipoRecogida = RECO_GRANADA;
    return true;
}

bool Rocket::recogerFuria(float cantidad) {
    if (furioso || barraFuria >= 1.f || cantidad <= 0.f)
        return false;
    sumarFuria(cantidad);
    recogidaPendiente = true;
    tipoRecogida = RECO_FURIA;
    return true;
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

short Rocket::consumirRecogida() {
    if (!recogidaPendiente) return -1;
    recogidaPendiente = false;
    return tipoRecogida;
}

bool Rocket::consumirPaso() {
    const bool r = pasoPendiente;
    pasoPendiente = false;
    return r;
}

bool Rocket::consumirAgacharse() {
    const bool r = agacharPendiente;
    agacharPendiente = false;
    return r;
}

bool Rocket::consumirAterrizaje() {
    const bool r = aterrizajePendiente;
    aterrizajePendiente = false;
    return r;
}

void Rocket::fijarCenital(bool c) {
    cenital = c;
}

void Rocket::aterrizarEn(float ySuperficie) {
    if (!enSuelo)
        aterrizajePendiente = true;
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
