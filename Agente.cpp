#include "Agente.h"
#include "Rocket.h"
#include <cmath>
#include <stdexcept>

namespace {
constexpr float ALCANCE_VISTA = 600.0f;
constexpr float UMBRAL_ALTO = 60.0f;
constexpr float INTENSIDAD_DASH = 0.8f;
constexpr float INTENSIDAD_GRANADA = 1.0f;
constexpr float VISTA_MIN = 0.1f;
constexpr float VISTA_MAX = 0.6f;
constexpr float TOLERANCIA = 1e-4f;

const float BASE[3][ACC_NUM] = {
      {  3.0f,  -1.0f,    1.0f,   -1.0f,     0.5f },
     {  1.5f,   3.0f,    1.5f,    1.0f,     1.0f },
      {  2.5f,   2.0f,    3.0f,    3.0f,    -1.0f }
};
const float RECARGA[ACC_NUM] = { 0.5f, 2.0f, 4.0f, 6.0f, 3.0f };
constexpr float CERCA = 150.0f;
constexpr float LEJOS = 300.0f;
constexpr float EMPATE = 1e-4f;

const float DURACION[ACC_NUM] = { 1.2f, 0.6f, 2.0f, 1.5f, 1.0f };

constexpr float ESCALA_PESO = 1.5f;
constexpr float BONO_HABITO = 1.5f;
constexpr int CONFIANZA_MIN = 10;
constexpr short TOPE_CONTEO = 30000;
constexpr float TIEMPO_HABITO = 2.0f;
constexpr float DISTANCIA_LEJOS = 300.0f;

const short CONTRA[HAB_NUM] = {
    ACC_GIRATORIO,
    ACC_TROMBA,
    ACC_CANON,
    ACC_ACOSAR
};
}

Agente::Agente()
    : tipoEstimulo(ESTIM_NINGUNO), intensidad(0.0f), distancia(0.0f), ladoRocket(1), alto(false),
      radioOido(400.0f), retraso(0.0f), pendiente(ESTIM_NINGUNO), intensidadPendiente(0.0f), espera(0.0f),
      agresividad(1.0f), factorRecarga(1.0f),
      actual(ACC_NINGUNA), restante(0.0f), duracion(0.0f), destinoX(0.0f), destinoY(0.0f),
      terminada(true), acierto(false), inicioPendiente(false),
      tasa(0.2f), estimuloPrevio(ESTIM_NINGUNO), tiempoAlto(0.0f), tiempoLejos(0.0f)
{
    recargas.fill(0.0f);
    sinUsar.fill(0.0f);
    bloqueada.fill(false);
    olvidarTodo();
}

void Agente::reiniciarIntento()
{
    tipoEstimulo = ESTIM_NINGUNO;
    intensidad = 0.0f;
    distancia = 0.0f;
    ladoRocket = 1;
    alto = false;
    pendiente = ESTIM_NINGUNO;
    intensidadPendiente = 0.0f;
    espera = 0.0f;

    recargas.fill(0.0f);
    sinUsar.fill(0.0f);
    bloqueada.fill(false);

    actual = ACC_NINGUNA;
    restante = duracion = 0.0f;
    destinoX = destinoY = 0.0f;
    terminada = true;
    acierto = false;
    inicioPendiente = false;

    estimuloPrevio = ESTIM_NINGUNO;
    tiempoAlto = tiempoLejos = 0.0f;
}

void Agente::percibir(const Rocket *r, float capX, float capY, float dt)
{
    if (r == nullptr) {
        tipoEstimulo = ESTIM_NINGUNO;
        intensidad = 0.0f;
        pendiente = ESTIM_NINGUNO;
        return;
    }

    float dx = r->getCentroX() - capX;
    float dy = r->getCentroY() - capY;
    distancia = std::sqrt(dx * dx + dy * dy);
    ladoRocket = dx >= 0.0f ? 1 : -1;
    alto = r->getCentroY() < capY - UMBRAL_ALTO;

    short tipo = ESTIM_NINGUNO;
    float inten = 0.0f;

    if (distancia <= ALCANCE_VISTA) {
        float v = 1.0f - distancia / ALCANCE_VISTA;
        inten = v < VISTA_MIN ? VISTA_MIN : (v > VISTA_MAX ? VISTA_MAX : v);
        tipo = ESTIM_VISTA;
    }
    float ruido = r->ruidoActual();
    if (ruido > 0.0f && distancia <= ruido && distancia <= radioOido && INTENSIDAD_DASH > inten) {
        inten = INTENSIDAD_DASH;
        tipo = ESTIM_DASH;
    }
    if (r->lanzoGranadaReciente() && INTENSIDAD_GRANADA > inten) {
        inten = INTENSIDAD_GRANADA;
        tipo = ESTIM_GRANADA;
    }

    if (tipo != pendiente) {
        pendiente = tipo;
        espera = retraso;
    } else {
        espera -= dt;
    }
    intensidadPendiente = inten;

    if (espera <= TOLERANCIA) {
        tipoEstimulo = pendiente;
        intensidad = intensidadPendiente;
    }
}

void Agente::fijarRetraso(float segundos)
{
    if (segundos < 0.0f) throw std::invalid_argument("Agente: el retraso no puede ser negativo");
    retraso = segundos;
}

void Agente::fijarRadioOido(float radio)
{
    if (radio <= 0.0f) throw std::invalid_argument("Agente: el radio de oido debe ser mayor que 0");
    radioOido = radio;
}

float Agente::puntaje(short a, short fase) const
{
    if (fase < 1 || fase > 3) throw std::out_of_range("Agente: la fase debe estar entre 1 y 3");
    if (a < 0 || a >= ACC_NUM) return NO_DISPONIBLE;

    float s = BASE[fase - 1][a];
    if (s < 0.0f) return NO_DISPONIBLE;

    s = (a == ACC_REPLEGARSE) ? s / agresividad : s * agresividad;

    if (distancia < CERCA) {
        if (a == ACC_ACOSAR) s += 2.0f;
        else if (a == ACC_GIRATORIO) s += 1.5f;
    } else if (distancia > LEJOS) {
        if (a == ACC_CANON) s += 2.0f;
        else if (a == ACC_ACOSAR) s -= 1.5f;
    }
    if (alto) {
        if (a == ACC_CANON) s += 1.0f;
        else if (a == ACC_ACOSAR) s -= 1.0f;
    }
    if (dashReciente()) {
        if (a == ACC_GIRATORIO) s += 2.0f;
        else if (a == ACC_ACOSAR) s += 1.0f;
    }
    if (granadaReciente()) {
        if (a == ACC_REPLEGARSE) s += 1.5f;
        else if (a == ACC_TROMBA) s += 1.0f;
    }
    return s + peso(a);
}

short Agente::decidir(short fase) const
{
    short mejor = ACC_NINGUNA;
    float mejorPuntaje = 0.0f;

    for (short a = 0; a < ACC_NUM; ++a) {
        if (bloqueada[a] || recargas[a] > 0.0f) continue;
        float s = puntaje(a, fase);
        if (s <= NO_DISPONIBLE) continue;

        bool gana = (mejor == ACC_NINGUNA) || (s > mejorPuntaje + EMPATE);
        if (!gana && s >= mejorPuntaje - EMPATE)
            gana = sinUsar[a] > sinUsar[mejor];
        if (gana) { mejor = a; mejorPuntaje = s; }
    }
    return mejor;
}

void Agente::iniciarRecarga(short accion)
{
    if (accion < 0 || accion >= ACC_NUM) throw std::out_of_range("Agente: accion desconocida");
    recargas[accion] = RECARGA[accion] * factorRecarga;
    sinUsar[accion] = 0.0f;
}

void Agente::actualizarRecargas(float dt)
{
    for (short a = 0; a < ACC_NUM; ++a) {
        recargas[a] = recargas[a] > dt ? recargas[a] - dt : 0.0f;
        sinUsar[a] += dt;
    }
}

bool Agente::estaLista(short accion) const
{
    return accion >= 0 && accion < ACC_NUM && !bloqueada[accion] && recargas[accion] <= 0.0f;
}

void Agente::bloquear(short accion, bool b)
{
    if (accion < 0 || accion >= ACC_NUM) throw std::out_of_range("Agente: accion desconocida");
    bloqueada[accion] = b;
}

void Agente::fijarAgresividad(float a)
{
    if (a <= 0.0f || a > 3.0f) throw std::invalid_argument("Agente: la agresividad debe estar en (0, 3]");
    agresividad = a;
}

void Agente::fijarFactorRecarga(float f)
{
    if (f <= 0.0f || f > 5.0f) throw std::invalid_argument("Agente: el factor de recarga debe estar en (0, 5]");
    factorRecarga = f;
}

void Agente::ejecutar(short accion, const Rocket *r)
{
    if (accion < 0 || accion >= ACC_NUM) throw std::out_of_range("Agente: accion desconocida");
    if (r == nullptr) throw std::invalid_argument("Agente: falta el objetivo");

    actual = accion;
    duracion = restante = DURACION[accion];
    destinoX = r->getCentroX();
    destinoY = r->getCentroY();
    terminada = false;
    acierto = false;
    inicioPendiente = true;
}

void Agente::actualizarAccion(float dt)
{
    if (terminada) return;
    restante -= dt;
    if (restante <= 0.0f) {
        restante = 0.0f;
        terminada = true;
    }
}

float Agente::getAvanceAccion() const
{
    if (duracion <= 0.0f) return 1.0f;
    return 1.0f - restante / duracion;
}

bool Agente::consumirInicio()
{
    bool r = inicioPendiente;
    inicioPendiente = false;
    return r;
}

void Agente::olvidarTodo()
{
    conteoHabitos.fill(0);
    pesosAccion.fill(0.0f);
    bonoAccion.fill(0.0f);
}

void Agente::registrarHabito(short habito)
{
    if (habito < 0 || habito >= HAB_NUM)
        throw std::out_of_range("Agente: habito desconocido");

    if (++conteoHabitos[habito] >= TOPE_CONTEO)
        for (short &c : conteoHabitos) c /= 2;
    recalcularBonos();
}

void Agente::observarHabitos(float dt)
{
    if (tipoEstimulo != estimuloPrevio) {
        if (tipoEstimulo == ESTIM_DASH) registrarHabito(HAB_DASH);
        else if (tipoEstimulo == ESTIM_GRANADA) registrarHabito(HAB_GRANADA);
        estimuloPrevio = tipoEstimulo;
    }
    tiempoAlto = alto ? tiempoAlto + dt : 0.0f;
    if (tiempoAlto >= TIEMPO_HABITO) { registrarHabito(HAB_ALTO); tiempoAlto = 0.0f; }
    tiempoLejos = distancia > DISTANCIA_LEJOS ? tiempoLejos + dt : 0.0f;
    if (tiempoLejos >= TIEMPO_HABITO) { registrarHabito(HAB_LEJOS); tiempoLejos = 0.0f; }
}

void Agente::recalcularBonos()
{
    bonoAccion.fill(0.0f);
    int total = 0;
    for (short c : conteoHabitos) total += c;
    if (total == 0) return;

    float confianza = total >= CONFIANZA_MIN ? 1.0f : static_cast<float>(total) / static_cast<float>(CONFIANZA_MIN);
    for (short h = 0; h < HAB_NUM; ++h)
        bonoAccion[CONTRA[h]] += BONO_HABITO * confianza * (static_cast<float>(conteoHabitos[h]) / static_cast<float>(total));
}

void Agente::resultado(short accion, bool acerto_)
{
    if (accion < 0 || accion >= ACC_NUM)
        throw std::out_of_range("Agente: accion desconocida");

    float objetivo = acerto_ ? 1.0f : -1.0f;
    pesosAccion[accion] += tasa * (objetivo - pesosAccion[accion]);
}

float Agente::peso(short accion) const
{
    if (accion < 0 || accion >= ACC_NUM) return 0.0f;
    return ESCALA_PESO * pesosAccion[accion] + bonoAccion[accion];
}

void Agente::fijarTasa(float t)
{
    if (t <= 0.0f || t > 1.0f)
        throw std::invalid_argument("Agente: la tasa debe estar en (0, 1]");
    tasa = t;
}

short Agente::conteo(short habito) const
{
    if (habito < 0 || habito >= HAB_NUM) return 0;
    return conteoHabitos[habito];
}

short Agente::habitoDominante() const
{
    short mejor = -1, max = 0;
    for (short h = 0; h < HAB_NUM; ++h)
        if (conteoHabitos[h] > max) { max = conteoHabitos[h]; mejor = h; }
    return mejor;
}
