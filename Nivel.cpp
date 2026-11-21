#include "Nivel.h"
#include "Entidad.h"
#include "Rocket.h"
#include "Enemigo.h"
#include "Proyectil.h"
#include "Eventos.h"

namespace {
constexpr float MARGEN_PIES = 8.f;
constexpr float TOLERANCIA_APOYO = 14.f;
constexpr float FURIA_POR_ENEMIGO = 0.25f;
constexpr float RADIO_MIN_EXPLOSION = 30.f;
constexpr float MARGEN_MUNDO = 200.f;
}

Nivel::Nivel()
    : tiempo(0.f), terminado(false), ganado(false), rocket(nullptr),
      anchoMundo(1280.f), altoMundo(720.f), cenital(false), ySuelo(1.0e9f), huecoIni(0.f), huecoFin(0.f),
      camaraX(0.f), apoyo(nullptr), seguroX(0.f), seguroY(0.f) {
    entidades.reserve(128);
    eventos.reserve(32);
    plataformas.reserve(8);
}

Nivel::~Nivel() {
    for (Entidad *e : entidades) {
        delete e;
    }
    entidades.clear();
    rocket = nullptr;
}

void Nivel::actualizar(float dt) {
    if (terminado) return;

    eliminarMuertas();
    tiempo += dt;

    for (std::size_t i = 0, n = entidades.size(); i < n; ++i) {
        if (entidades[i]->estaViva()) entidades[i]->actualizar(dt);
    }

    disparosEnemigos();
    if (rocket) {
        if (cenital) bloquearContraEstructuras();
        else resolverApoyo();
    }
    limitarAlMundo();
    colisiones();
    explosiones();
    notificarGolpe();
}

void Nivel::agregarEntidad(Entidad *e) {
    if (!e) return;
    entidades.push_back(e);

    const short c = e->clase();
    if (c == CLASE_ROCKET && !rocket) {
        rocket = static_cast<Rocket *>(e);
        rocket->fijarCenital(cenital);
        seguroX = rocket->getX();
        seguroY = rocket->getY();
    } else if (c == CLASE_ENEMIGO || c == CLASE_CAPITAN) {
        Enemigo *en = static_cast<Enemigo *>(e);
        en->fijarCenital(cenital);
        en->fijarObjetivo(rocket);
    } else if (c == CLASE_PROYECTIL && !cenital) {
        Proyectil *p = static_cast<Proyectil *>(e);
        if (!p->esBombeado()) p->fijarSuelo(ySuelo);
    }
}

void Nivel::disparosEnemigos() {
    if (!rocket) return;
    for (std::size_t i = 0, n = entidades.size(); i < n; ++i) {
        Entidad *e = entidades[i];
        if (e->clase() != CLASE_ENEMIGO || !e->estaViva()) continue;
        Enemigo *en = static_cast<Enemigo *>(e);
        if (!en->listoParaDisparar()) continue;
        Proyectil *p = en->disparar(rocket->getCentroX(), rocket->getCentroY());
        if (p) {
            agregarEntidad(p);
            emitirEvento(EV_DISPARO_ENEMIGO);
        }
    }
}

void Nivel::resolverApoyo() {
    const float izq = rocket->getX() + MARGEN_PIES;
    const float der = rocket->getX() + rocket->getAncho() - MARGEN_PIES;
    const float cx = rocket->getCentroX();
    const bool haySuelo = !(cx > huecoIni && cx < huecoFin);

    if (rocket->estaEnSuelo()) {
        if (apoyo) {
            if (der > apoyo->getX() && izq < apoyo->getX() + apoyo->getAncho()) {
                rocket->aterrizarEn(apoyo->getY());
            } else {
                apoyo = nullptr;
                rocket->soltarSuelo();
            }
        } else if (haySuelo) {
            seguroX = rocket->getX();
            seguroY = rocket->getY();
        } else {
            rocket->soltarSuelo();
        }
    } else if (rocket->getVelY() >= 0.f) {
        const float pies = rocket->getY() + rocket->getAlto();
        const float piesAntes = rocket->getYAnterior() + rocket->getAlto();
        bool encontro = false;
        float mejor = 0.f;
        Entidad *mejorApoyo = nullptr;

        for (Entidad *p : plataformas) {
            if (!(der > p->getX() && izq < p->getX() + p->getAncho())) continue;
            const float tope = p->getY();
            if (piesAntes <= tope + TOLERANCIA_APOYO && pies >= tope && (!encontro || tope < mejor)) {
                encontro = true;
                mejor = tope;
                mejorApoyo = p;
            }
        }
        if (haySuelo && pies >= ySuelo && piesAntes <= ySuelo + 3.f * TOLERANCIA_APOYO && (!encontro || ySuelo < mejor)) {
            encontro = true;
            mejor = ySuelo;
            mejorApoyo = nullptr;
        }
        if (encontro) {
            rocket->aterrizarEn(mejor);
            apoyo = mejorApoyo;
        }
    }

    if (rocket->getY() > altoMundo + 40.f) {
        rocket->perderVida();
        rocket->reaparecerEn(seguroX, seguroY);
        apoyo = nullptr;
        emitirEvento(EV_CAIDA);
    }
}

void Nivel::bloquearContraEstructuras() {
    for (Entidad *e : entidades) {
        if (e->clase() != CLASE_ESTRUCTURA || !e->esSolida() || !rocket->colisiona(e)) continue;

        const float nx = rocket->getX();
        const float ny = rocket->getY();
        rocket->setPosicion(rocket->getXAnterior(), ny);
        if (!rocket->colisiona(e)) continue;
        rocket->setPosicion(nx, rocket->getYAnterior());
        if (!rocket->colisiona(e)) continue;
        rocket->setPosicion(rocket->getXAnterior(), rocket->getYAnterior());
    }
}

void Nivel::limitarAlMundo() {
    if (rocket) {
        float nx = rocket->getX();
        float ny = rocket->getY();
        if (nx < 0.f) nx = 0.f;
        else if (nx > anchoMundo - rocket->getAncho()) nx = anchoMundo - rocket->getAncho();
        if (cenital) {
            if (ny < 0.f) ny = 0.f;
            else if (ny > altoMundo - rocket->getAlto()) ny = altoMundo - rocket->getAlto();
        }
        rocket->setPosicion(nx, ny);
    }

    for (Entidad *e : entidades) {
        if (e->clase() != CLASE_PROYECTIL || !e->estaViva()) continue;
        if (e->getX() < -MARGEN_MUNDO || e->getX() > anchoMundo + MARGEN_MUNDO ||
            e->getY() > altoMundo + MARGEN_MUNDO || e->getY() < -4.f * MARGEN_MUNDO) {
            e->matar();
        }
    }
}

void Nivel::colisiones() {
    const std::size_t n = entidades.size();
    for (std::size_t i = 0; i < n; ++i) {
        Entidad *a = entidades[i];
        if (!a->estaViva()) continue;
        const bool aFija = a->clase() == CLASE_ESTRUCTURA;

        for (std::size_t j = i + 1; j < n; ++j) {
            Entidad *b = entidades[j];
            if (aFija && b->clase() == CLASE_ESTRUCTURA) continue;
            if (!a->colisiona(b)) continue;
            a->alChocar(*b);
            b->alChocar(*a);
            if (!a->estaViva()) break;
        }
    }
}

void Nivel::explosiones() {
    for (std::size_t i = 0, n = entidades.size(); i < n; ++i) {
        Entidad *e = entidades[i];
        if (e->clase() != CLASE_PROYECTIL) continue;
        Proyectil *p = static_cast<Proyectil *>(e);
        if (!p->consumirExplosion()) continue;

        if (p->getRadioExplosion() >= RADIO_MIN_EXPLOSION) emitirEvento(EV_EXPLOSION);
        explosionEn(p->getCentroX(), p->getCentroY(), p->getRadioExplosion(), p->getDanio(), p->esDeRocket());
    }
}

void Nivel::explosionEn(float cx, float cy, float radio, float danio, bool deRocket) {
    const float r2 = radio * radio;
    for (Entidad *e : entidades) {
        const short c = e->clase();
        if (!e->estaViva() || c == CLASE_PROYECTIL) continue;
        if (deRocket == (c == CLASE_ROCKET)) continue;

        float dx = 0.f;
        if (cx < e->getX()) dx = e->getX() - cx;
        else if (cx > e->getX() + e->getAncho()) dx = cx - (e->getX() + e->getAncho());
        float dy = 0.f;
        if (cy < e->getY()) dy = e->getY() - cy;
        else if (cy > e->getY() + e->getAlto()) dy = cy - (e->getY() + e->getAlto());
        if (dx * dx + dy * dy > r2) continue;

        e->recibirDanio(danio);
        if (c == CLASE_ENEMIGO && !e->estaViva()) {
            emitirEvento(EV_ENEMIGO_CAE);
            if (rocket) rocket->sumarFuria(FURIA_POR_ENEMIGO);
        }
    }
}

void Nivel::notificarGolpe() {
    if (rocket && rocket->consumirGolpe()) emitirEvento(EV_GOLPE_A_ROCKET);
}

void Nivel::eliminarMuertas() {
    std::size_t k = 0;
    for (std::size_t i = 0; i < entidades.size(); ++i) {
        Entidad *e = entidades[i];
        if (e->estaViva() || e == rocket) {
            entidades[k++] = e;
        } else {
            alBorrar(e);
            delete e;
        }
    }
    entidades.resize(k);
}

void Nivel::alBorrar(Entidad *e) {
    if (e == apoyo) apoyo = nullptr;
    for (std::size_t i = 0; i < plataformas.size(); ++i) {
        if (plataformas[i] == e) {
            plataformas[i] = plataformas.back();
            plataformas.pop_back();
            break;
        }
    }
}

short Nivel::contarVivos(short clase) const {
    short n = 0;
    for (const Entidad *e : entidades) {
        if (e->clase() == clase && e->estaViva()) ++n;
    }
    return n;
}

void Nivel::emitirEvento(short codigo) {
    eventos.push_back(codigo);
}

void Nivel::terminar(bool gano) {
    if (terminado) return;
    terminado = true;
    ganado = gano;
    emitirEvento(gano ? EV_VICTORIA : EV_DERROTA);
}

void Nivel::registrarPlataforma(Entidad *p) {
    if (p) plataformas.push_back(p);
}

Rocket *Nivel::jugador() const {
    return rocket;
}

void Nivel::jugadorMueve(char dir) {
    if (rocket && !terminado) rocket->mover(dir);
}

void Nivel::jugadorSalta() {
    if (!rocket || terminado) return;
    if (rocket->saltar()) {
        apoyo = nullptr;
        emitirEvento(EV_SALTO);
    }
}

void Nivel::jugadorDispara(float objX, float objY) {
    if (!rocket || terminado) return;
    Proyectil *p = rocket->disparar(objX, objY);
    if (p) {
        agregarEntidad(p);
        emitirEvento(EV_DISPARO_ROCKET);
    }
}

void Nivel::jugadorLanzaGranada(float objX, float objY) {
    if (!rocket || terminado) return;
    Proyectil *p = rocket->lanzarGranada(objX, objY);
    if (p) {
        agregarEntidad(p);
        emitirEvento(EV_GRANADA);
    }
}

void Nivel::jugadorDash() {
    if (!rocket || terminado) return;
    if (rocket->dash(rocket->getDireccion())) emitirEvento(EV_DASH);
}

void Nivel::jugadorActivaFuria() {
    if (!rocket || terminado) return;
    if (rocket->activarFuria()) emitirEvento(EV_FURIA_ACTIVADA);
}

const std::vector<Entidad *> &Nivel::obtenerEntidades() const {
    return entidades;
}

void Nivel::tomarEventos(std::vector<short> &destino) {
    destino.clear();
    destino.swap(eventos);
}

float Nivel::tiempoTranscurrido() const { return tiempo; }
bool Nivel::estaTerminado() const { return terminado; }
bool Nivel::fueGanado() const { return ganado; }
bool Nivel::esCenital() const { return cenital; }
float Nivel::getAnchoMundo() const { return anchoMundo; }
float Nivel::getAltoMundo() const { return altoMundo; }
float Nivel::getCamaraX() const { return camaraX; }
float Nivel::getSuelo() const { return ySuelo; }
