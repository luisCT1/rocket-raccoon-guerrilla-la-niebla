#include "EscenaEmboscada.h"
#include "HojaSprites.h"

#include <QImage>
#include <QKeyEvent>
#include <QtMath>
#include <algorithm>

namespace {
//El sendero de tierra del fondo queda en esta linea. Los pies y el tablon se apoyan ahi.
constexpr float kSuelo = 620.f;
//El tablon del dibujo queda mas abajo que la cuerda. Este ajuste lo sube hasta el sendero.
constexpr float kLevantaPuente = 56.f;
constexpr float kLabioIzq = 548.f;
constexpr float kLabioDer = 1167.f;
constexpr float kCentroHueco = (kLabioIzq + kLabioDer) * 0.5f;
constexpr float kGravedad = 1700.f;
constexpr float kSalto = -560.f;
constexpr float kVel = 210.f;
constexpr float kAltoMapache = 145.f;
constexpr int kAnchoFondo = 1672;
constexpr int kAltoFondo = 941;

int piesDelCuadro(const QPixmap &pm) {
    const QImage img = pm.toImage();
    const int x0 = img.width() * 35 / 100;
    const int x1 = img.width() * 65 / 100;
    QVector<int> bajos;
    for (int x = x0; x < x1; ++x) {
        for (int y = img.height() - 1; y >= 0; --y) {
            if (qAlpha(img.pixel(x, y)) > 40) {
                bajos.push_back(y);
                break;
            }
        }
    }
    if (bajos.isEmpty()) return qMax(0, img.height() - 1);
    std::sort(bajos.begin(), bajos.end());
    return bajos.at(bajos.size() / 2);
}

QVector<int> medirPies(const QVector<QPixmap> &cuadros) {
    QVector<int> pies;
    pies.reserve(cuadros.size());
    for (const QPixmap &cuadro : cuadros) pies.push_back(piesDelCuadro(cuadro));
    return pies;
}

int posteEn(const QImage &img, int x0, int x1) {
    int mejor = x0;
    int puntaje = -1;
    const int alto = img.height() * 55 / 100;
    for (int x = x0; x < x1; ++x) {
        int suma = 0;
        for (int y = 0; y < alto; ++y) {
            if (qAlpha(img.pixel(x, y)) > 100) ++suma;
        }
        if (suma > puntaje) {
            puntaje = suma;
            mejor = x;
        }
    }
    return mejor;
}

QVector<int> cubiertaDe(const QImage &img) {
    const int w = img.width();
    const int h = img.height();
    QVector<int> cubierta(w, -1);
    for (int x = 0; x < w; ++x) {
        int mejorInicio = -1;
        int mejorFin = -1;
        int inicio = -1;
        for (int y = 0; y <= h; ++y) {
            const bool opaco = y < h && qAlpha(img.pixel(x, y)) > 80;
            if (opaco && inicio < 0) inicio = y;
            if (!opaco && inicio >= 0) {
                const int fin = y - 1;
                if ((fin - inicio) >= 20 && inicio > h * 0.45 && fin > mejorFin) {
                    mejorInicio = inicio;
                    mejorFin = fin;
                }
                inicio = -1;
            }
        }
        cubierta[x] = mejorInicio;
    }
    return cubierta;
}

int pisoDelCuadro(const QPixmap &pm) {
    const QImage img = pm.toImage();
    int bajo = 0;
    for (int x = 0; x < img.width(); x += 2) {
        for (int y = img.height() - 1; y >= 0; --y) {
            if (qAlpha(img.pixel(x, y)) > 40) {
                if (y > bajo) bajo = y;
                break;
            }
        }
    }
    return qMax(1, bajo);
}

void cargarTira(const QPixmap &hoja, const QVector<QRect> &rects, float escala,
                QVector<QPixmap> &salida, QVector<int> &pies) {
    salida.clear();
    pies.clear();
    for (const QRect &r : rects) {
        const QPixmap recorte = hoja.copy(r);
        const int dw = qMax(1, qRound(recorte.width() * escala));
        const int dh = qMax(1, qRound(recorte.height() * escala));
        const QPixmap cuadro = recorte.scaled(dw, dh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        salida.push_back(cuadro);
        pies.push_back(pisoDelCuadro(cuadro));
    }
}

QVector<QRect> cuadrosSoldadoCamina() {
    return {
        QRect(112, 20, 292, 368),
        QRect(516, 36, 288, 352),
        QRect(896, 36, 312, 348),
        QRect(1284, 20, 284, 368)
    };
}

QVector<QRect> cuadrosSoldadoDispara() {
    return {
        QRect(100, 780, 336, 364),
        QRect(608, 784, 340, 360),
        QRect(1352, 796, 348, 348)
    };
}

QVector<QRect> cuadrosSoldadoMuere() {
    return {
        QRect(148, 1220, 348, 288),
        QRect(828, 1200, 416, 320),
        QRect(1468, 1384, 464, 120),
        QRect(2136, 1368, 468, 168)
    };
}

QVector<QRect> cuadrosJineteGalope() {
    return {
        QRect(20, 28, 512, 456),
        QRect(568, 28, 512, 468),
        QRect(1112, 28, 516, 468),
        QRect(1668, 28, 512, 456),
        QRect(2208, 28, 500, 468)
    };
}

QVector<QRect> cuadrosJineteMuere() {
    return {
        QRect(120, 1056, 520, 440),
        QRect(796, 1100, 496, 392),
        QRect(1560, 1180, 500, 320),
        QRect(2160, 1356, 504, 132)
    };
}

QVector<QRect> cuadrosCarretaRueda() {
    return {
        QRect(112, 424, 516, 320),
        QRect(780, 428, 520, 320),
        QRect(1472, 428, 512, 312),
        QRect(2128, 428, 516, 312)
    };
}

QVector<QRect> cuadrosCarretaHerida() {
    return {
        QRect(96, 804, 540, 316),
        QRect(812, 804, 508, 316)
    };
}

QVector<QRect> cuadrosCarretaExplota() {
    return {
        QRect(136, 1164, 496, 344),
        QRect(780, 1232, 580, 268),
        QRect(1488, 1300, 524, 204),
        QRect(2152, 1304, 496, 204)
    };
}

float anclaDe(const QVector<int> &cubierta) {
    QVector<int> muestras;
    const int w = cubierta.size();
    auto tomar = [&](int a, int b) {
        for (int x = a; x < b; ++x) {
            if (cubierta.at(x) >= 0) muestras.push_back(cubierta.at(x));
        }
    };
    tomar(w * 12 / 100, w * 22 / 100);
    tomar(w * 78 / 100, w * 88 / 100);
    if (muestras.isEmpty()) return 130.f;
    std::sort(muestras.begin(), muestras.end());
    return float(muestras.at(muestras.size() / 2));
}

QVector<QRect> cuadrosCaminar() {
    return {
        QRect(49, 36, 255, 224),
        QRect(349, 36, 256, 224),
        QRect(651, 36, 249, 224),
        QRect(941, 36, 253, 224),
        QRect(1244, 36, 251, 224)
    };
}

QVector<QRect> cuadrosSaltar() {
    return {
        QRect(60, 277, 199, 257),
        QRect(368, 277, 191, 257),
        QRect(660, 277, 190, 257),
        QRect(953, 277, 186, 257),
        QRect(1247, 277, 197, 257)
    };
}

QVector<QRect> cuadrosDisparar() {
    return {
        QRect(46, 580, 300, 229),
        QRect(369, 580, 320, 229),
        QRect(700, 580, 300, 229)
    };
}

QVector<QRect> cuadrosImpulso() {
    return {
        QRect(1023, 580, 300, 229),
        QRect(1456, 580, 280, 229),
        QRect(1736, 580, 320, 229),
        QRect(2412, 580, 280, 229)
    };
}

QVector<QRect> cuadrosFuria() {
    return {
        QRect(1023, 868, 260, 245),
        QRect(1440, 868, 300, 245),
        QRect(1762, 868, 310, 245),
        QRect(2436, 868, 210, 245)
    };
}

QVector<QRect> cuadrosHerido() {
    return {
        QRect(55, 1203, 251, 283),
        QRect(378, 1203, 218, 283),
        QRect(691, 1203, 181, 283)
    };
}

QVector<QRect> cuadrosCaido() {
    return {
        QRect(1114, 1203, 236, 283),
        QRect(1427, 1203, 261, 283),
        QRect(1753, 1203, 280, 283),
        QRect(2105, 1203, 251, 283),
        QRect(2410, 1203, 280, 283)
    };
}

QVector<QRect> cuadrosPuente() {
    return {
        QRect(47, 624, 496, 236),
        QRect(569, 624, 519, 236),
        QRect(1116, 624, 521, 236),
        QRect(1664, 624, 520, 236),
        QRect(2214, 624, 498, 236)
    };
}
}

EscenaEmboscada::EscenaEmboscada() {
    setSceneRect(0, 0, kAnchoFondo, kAltoFondo);
    cargarSprites();

    fondo = addPixmap(QPixmap(QStringLiteral(":/nivel1/fondo_emboscada.png")));
    fondo->setZValue(0);

    puente = addPixmap(marcosPuente.value(2).pix);
    puente->setZValue(1);

    mapache = addPixmap(caminar.value(0));
    mapache->setZValue(4);

    prepararConvoy();
    ponerPuente(0.f);
    ponerMapache();
}

void EscenaEmboscada::cargarSprites() {
    const QPixmap hoja(QStringLiteral(":/nivel1/rocket_sheet.png"));
    const QPixmap hojaPuente(QStringLiteral(":/nivel1/puente_oscilante.png"));

    caminar = cortarHoja(hoja, cuadrosCaminar(), kAltoMapache);
    saltar = cortarHoja(hoja, cuadrosSaltar(), kAltoMapache);
    disparar = cortarHoja(hoja, cuadrosDisparar(), kAltoMapache);
    impulso = cortarHoja(hoja, cuadrosImpulso(), kAltoMapache);
    furia = cortarHoja(hoja, cuadrosFuria(), kAltoMapache);
    herido = cortarHoja(hoja, cuadrosHerido(), kAltoMapache);
    caido = cortarHoja(hoja, cuadrosCaido(), kAltoMapache);
    piesCaminar = medirPies(caminar);
    piesSaltar = medirPies(saltar);
    piesDisparar = medirPies(disparar);
    piesImpulso = medirPies(impulso);
    piesFuria = medirPies(furia);
    piesHerido = medirPies(herido);
    piesCaido = medirPies(caido);

    //Los postes del cuadro central marcan el ancho real del hueco entre los dos riscos.
    const QVector<QRect> cortes = cuadrosPuente();
    struct Crudo { QImage img; int izq; int der; float ancla; QVector<int> cubierta; };
    QVector<Crudo> crudos;
    for (const QRect &r : cortes) {
        Crudo c;
        c.img = hojaPuente.copy(r).toImage();
        c.izq = posteEn(c.img, 0, qMin(140, c.img.width()));
        c.der = posteEn(c.img, qMax(0, c.img.width() - 140), c.img.width());
        c.cubierta = cubiertaDe(c.img);
        c.ancla = anclaDe(c.cubierta);
        crudos.push_back(c);
    }
    const int central = crudos.size() > 2 ? 2 : 0;
    const float separacion = float(qMax(1, crudos.at(central).der - crudos.at(central).izq));
    escalaPuente = (kLabioDer - kLabioIzq) / separacion;
    for (const Crudo &c : crudos) {
        MarcoPuente marco;
        marco.cubierta = c.cubierta;
        marco.medio = (c.izq + c.der) * 0.5f;
        marco.ancla = c.ancla;
        const int dw = qRound(c.img.width() * escalaPuente);
        const int dh = qRound(c.img.height() * escalaPuente);
        marco.pix = QPixmap::fromImage(c.img).scaled(dw, dh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        marcosPuente.push_back(marco);
    }

    const QPixmap hojaSoldado(QStringLiteral(":/nivel1/soldado_sheet.png"));
    const QPixmap hojaJinete(QStringLiteral(":/nivel1/jinete_correo.png"));
    const QPixmap hojaCarreta(QStringLiteral(":/nivel1/carreta_convoy.png"));
    const float escalaSoldado = 132.f / 360.f;
    const float escalaJinete = 188.f / 460.f;
    const float escalaCarreta = 150.f / 316.f;
    cargarTira(hojaSoldado, cuadrosSoldadoCamina(), escalaSoldado, soldadoCamina, piesSoldadoCamina);
    cargarTira(hojaSoldado, cuadrosSoldadoDispara(), escalaSoldado, soldadoDispara, piesSoldadoDispara);
    cargarTira(hojaSoldado, cuadrosSoldadoMuere(), escalaSoldado, soldadoMuere, piesSoldadoMuere);
    cargarTira(hojaJinete, cuadrosJineteGalope(), escalaJinete, jineteGalope, piesJineteGalope);
    cargarTira(hojaJinete, cuadrosJineteMuere(), escalaJinete, jineteMuere, piesJineteMuere);
    cargarTira(hojaCarreta, cuadrosCarretaRueda(), escalaCarreta, carretaRueda, piesCarretaRueda);
    cargarTira(hojaCarreta, cuadrosCarretaHerida(), escalaCarreta, carretaHerida, piesCarretaHerida);
    cargarTira(hojaCarreta, cuadrosCarretaExplota(), escalaCarreta, carretaExplota, piesCarretaExplota);
}

void EscenaEmboscada::pulsar(int tecla) {
    teclas.insert(tecla);
    if (!vivo) return;

    if ((tecla == Qt::Key_W || tecla == Qt::Key_Up) && !enAire && pose != Pose::Caido) {
        enAire = true;
        vy = kSalto;
        pose = Pose::Salta;
        anim = 0.f;
    }
    if (tecla == Qt::Key_Space && recargaDisparo <= 0.f && pose != Pose::Impulso && pose != Pose::Caido) {
        dispararFusil();
    }
    if (tecla == Qt::Key_G && pose != Pose::Caido) {
        lanzarGranada();
    }
    if (tecla == Qt::Key_Shift && pose != Pose::Impulso && pose != Pose::Caido && !enAire) {
        pose = Pose::Impulso;
        poseTimer = 0.28f;
        anim = 0.f;
    }
    if (tecla == Qt::Key_F) {
        furiaTimer = 5.f;
    }
}

void EscenaEmboscada::soltar(int tecla) {
    teclas.remove(tecla);
}

int EscenaEmboscada::cuadroPuente() const {
    if (marcosPuente.size() < 4) return 0;
    //Los cuadros de los extremos despegan un poste del risco. Se usan los tres del medio.
    const float onda = qSin(tiempo * 1.6f);
    const int cuadro = 1 + qRound((onda + 1.f));
    return qBound(1, cuadro, 3);
}

float EscenaEmboscada::origenXPuente(int cuadro) const {
    return kCentroHueco - marcosPuente.at(cuadro).medio * escalaPuente;
}

float EscenaEmboscada::origenYPuente(int cuadro, float osc) const {
    return kSuelo + osc - marcosPuente.at(cuadro).ancla * escalaPuente - kLevantaPuente;
}

float EscenaEmboscada::sueloEn(float px, float osc) const {
    if (marcosPuente.isEmpty()) return kSuelo;
    const int cuadro = cuadroPuente();
    const MarcoPuente &marco = marcosPuente.at(cuadro);
    const float local = (px - origenXPuente(cuadro)) / escalaPuente;
    //Fuera de los postes el piso es el sendero. Encima de la cuerda no se camina.
    if (local < 100.f || local > marco.cubierta.size() - 100.f) return kSuelo;

    const int x0 = qBound(0, int(qRound(local)), marco.cubierta.size() - 1);
    int izquierda = x0;
    int derecha = x0;
    while (izquierda >= 0 && marco.cubierta.at(izquierda) < 0) --izquierda;
    while (derecha < marco.cubierta.size() && marco.cubierta.at(derecha) < 0) ++derecha;
    int alto = -1;
    if (izquierda >= 0 && derecha < marco.cubierta.size() && derecha != izquierda) {
        const float t = float(x0 - izquierda) / float(derecha - izquierda);
        alto = int(marco.cubierta.at(izquierda) * (1.f - t) + marco.cubierta.at(derecha) * t);
    } else if (izquierda >= 0) {
        alto = marco.cubierta.at(izquierda);
    } else if (derecha < marco.cubierta.size()) {
        alto = marco.cubierta.at(derecha);
    }
    if (alto < 0) return kSuelo;
    if (alto < int(marco.ancla) - 12) alto = int(marco.ancla);
    return origenYPuente(cuadro, osc) + alto * escalaPuente;
}

void EscenaEmboscada::ponerPuente(float osc) {
    if (marcosPuente.isEmpty() || !puente) return;
    const int cuadro = cuadroPuente();
    puente->setPixmap(marcosPuente.at(cuadro).pix);
    puente->setPos(origenXPuente(cuadro), origenYPuente(cuadro, osc));
}

void EscenaEmboscada::ponerMapache() {
    if (!mapache) return;
    const QVector<QPixmap> &hoja = hojaDe(pose);
    if (hoja.isEmpty()) return;

    int cuadro = 0;
    if (pose == Pose::Camina) {
        cuadro = int(anim * 8.f) % hoja.size();
    } else if (pose == Pose::Salta) {
        if (vy < -80.f) cuadro = 1;
        else if (vy > 80.f) cuadro = 3;
        else cuadro = 2;
        cuadro = qMin(cuadro, hoja.size() - 1);
    } else if (pose == Pose::Dispara || pose == Pose::Furia || pose == Pose::Impulso || pose == Pose::Herido) {
        cuadro = qMin(int(anim * 12.f), hoja.size() - 1);
    } else if (pose == Pose::Caido) {
        cuadro = hoja.size() - 1;
    }

    QPixmap sprite = hoja.at(cuadro);
    if (mira < 0) {
        sprite = sprite.transformed(QTransform().scale(-1, 1), Qt::SmoothTransformation);
    }
    mapache->setPixmap(sprite);
    const QVector<int> &pies = piesDe(pose);
    const int pie = cuadro < pies.size() ? pies.at(cuadro) : sprite.height();
    mapache->setPos(x - sprite.width() * 0.5, y - pie);
}

const QVector<int> &EscenaEmboscada::piesDe(Pose cual) const {
    switch (cual) {
    case Pose::Salta: return piesSaltar;
    case Pose::Dispara: return piesDisparar;
    case Pose::Impulso: return piesImpulso;
    case Pose::Furia: return piesFuria;
    case Pose::Herido: return piesHerido;
    case Pose::Caido: return piesCaido;
    case Pose::Quieto:
    case Pose::Camina:
        break;
    }
    return piesCaminar;
}

const QVector<QPixmap> &EscenaEmboscada::hojaDe(Pose cual) const {
    switch (cual) {
    case Pose::Salta: return saltar;
    case Pose::Dispara: return disparar;
    case Pose::Impulso: return impulso;
    case Pose::Furia: return furia;
    case Pose::Herido: return herido;
    case Pose::Caido: return caido;
    case Pose::Quieto:
    case Pose::Camina:
        break;
    }
    return caminar;
}

void EscenaEmboscada::prepararConvoy() {
    auto crear = [this](int tipo, float px, float vel, int vida, float alto, float ancho) {
        Actor actor;
        actor.tipo = tipo;
        actor.x = px;
        actor.y = kSuelo;
        actor.vel = vel;
        actor.vida = vida;
        actor.vidaMax = vida;
        actor.alto = alto;
        actor.ancho = ancho;
        actor.mira = vel < 0.f ? -1 : 1;
        actor.dibujo = addPixmap(tipo == 2 ? carretaRueda.value(0)
                                : tipo == 1 ? jineteGalope.value(0)
                                            : soldadoCamina.value(0));
        actor.dibujo->setZValue(3);
        convoy.push_back(actor);
    };
    crear(0, 400.f, -36.f, 2, 132.f, 46.f);
    crear(0, 980.f, -34.f, 2, 132.f, 46.f);
    crear(2, 1380.f, -28.f, 4, 150.f, 120.f);
    crear(1, 1580.f, -120.f, 2, 188.f, 70.f);
    const float osc = 0.f;
    for (Actor &actor : convoy) {
        actor.y = sueloEn(actor.x, osc);
        ponerActor(actor);
    }
}

void EscenaEmboscada::ponerActor(Actor &actor) {
    if (!actor.dibujo) return;
    const QVector<QPixmap> *hoja = &soldadoCamina;
    const QVector<int> *pies = &piesSoldadoCamina;
    bool bucle = true;
    if (actor.tipo == 0) {
        if (!actor.vivo) {
            hoja = &soldadoMuere;
            pies = &piesSoldadoMuere;
            bucle = false;
        } else if (actor.poseT > 0.f) {
            hoja = &soldadoDispara;
            pies = &piesSoldadoDispara;
            bucle = false;
        }
    } else if (actor.tipo == 1) {
        if (!actor.vivo) {
            hoja = &jineteMuere;
            pies = &piesJineteMuere;
            bucle = false;
        } else {
            hoja = &jineteGalope;
            pies = &piesJineteGalope;
        }
    } else {
        if (!actor.vivo) {
            hoja = &carretaExplota;
            pies = &piesCarretaExplota;
            bucle = false;
        } else if (actor.vida <= actor.vidaMax / 2) {
            hoja = &carretaHerida;
            pies = &piesCarretaHerida;
        } else {
            hoja = &carretaRueda;
            pies = &piesCarretaRueda;
        }
    }
    if (hoja->isEmpty()) return;
    int cuadro = int(actor.anim * (actor.tipo == 1 ? 12.f : 8.f));
    if (bucle) cuadro %= hoja->size();
    else cuadro = qMin(cuadro, hoja->size() - 1);
    QPixmap sprite = hoja->at(cuadro);
    if (actor.mira < 0) {
        sprite = sprite.transformed(QTransform().scale(-1, 1), Qt::SmoothTransformation);
    }
    actor.dibujo->setPixmap(sprite);
    const int pie = cuadro < pies->size() ? pies->at(cuadro) : sprite.height();
    actor.dibujo->setPos(actor.x - sprite.width() * 0.5, actor.y - pie);
}

void EscenaEmboscada::avanzarConvoy(float dt, float osc) {
    for (Actor &actor : convoy) {
        actor.anim += dt;
        if (actor.poseT > 0.f) actor.poseT -= dt;
        if (actor.recarga > 0.f) actor.recarga -= dt;
        if (actor.vivo) {
            const bool disparando = actor.tipo == 0 && actor.poseT > 0.f;
            if (!disparando) actor.x += actor.vel * dt;
            if (actor.x < 80.f) actor.x = 1640.f;
            if (actor.tipo == 0 && vivo && actor.recarga <= 0.f) {
                const float dx = x - actor.x;
                if (qAbs(dx) < 620.f && qAbs(y - actor.y) < 90.f) {
                    actor.mira = dx < 0.f ? -1 : 1;
                    actor.poseT = 0.42f;
                    actor.anim = 0.f;
                    actor.recarga = 1.35f;
                    Bala bala;
                    bala.delJugador = false;
                    bala.x = actor.x + actor.mira * 48.f;
                    bala.y = actor.y - actor.alto * 0.46f;
                    bala.vx = actor.mira * 460.f;
                    bala.vy = 0.f;
                    bala.dibujo = addEllipse(-4, -4, 8, 8, QPen(Qt::NoPen), QBrush(QColor(180, 40, 36)));
                    bala.dibujo->setZValue(5);
                    bala.dibujo->setPos(bala.x, bala.y);
                    balas.push_back(bala);
                }
            }
        }
        actor.y = sueloEn(actor.x, osc);
        ponerActor(actor);
    }
}

void EscenaEmboscada::dispararFusil() {
    pose = furiaTimer > 0.f ? Pose::Furia : Pose::Dispara;
    poseTimer = 0.32f;
    recargaDisparo = furiaTimer > 0.f ? 0.18f : 0.34f;
    anim = 0.f;

    Bala bala;
    bala.delJugador = true;
    bala.granada = false;
    bala.x = x + mira * 46.f;
    bala.y = y - 72.f;
    bala.vx = mira * (furiaTimer > 0.f ? 780.f : 640.f);
    bala.vy = 0.f;
    bala.dano = furiaTimer > 0.f ? 2 : 1;
    const qreal radio = furiaTimer > 0.f ? 6.0 : 4.0;
    bala.dibujo = addEllipse(-radio, -radio, radio * 2, radio * 2,
                             QPen(Qt::NoPen), QBrush(QColor(255, 214, 90)));
    bala.dibujo->setZValue(5);
    bala.dibujo->setPos(bala.x, bala.y);
    balas.push_back(bala);
}

void EscenaEmboscada::lanzarGranada() {
    if (granadas <= 0 || pose == Pose::Caido) return;
    --granadas;
    pose = Pose::Dispara;
    poseTimer = 0.28f;
    anim = 0.f;

    Bala bala;
    bala.delJugador = true;
    bala.granada = true;
    bala.vx = mira;
    bala.x0 = x + mira * 24.f;
    bala.y0 = y - 80.f;
    bala.x = bala.x0;
    bala.y = bala.y0;
    bala.dibujo = addEllipse(-8, -8, 16, 16, QPen(Qt::NoPen), QBrush(QColor(70, 90, 48)));
    bala.dibujo->setZValue(5);
    bala.dibujo->setPos(bala.x, bala.y);
    balas.push_back(bala);
}

void EscenaEmboscada::explotar(float gx, float gy) {
    Destello destello;
    destello.dibujo = addEllipse(-34, -34, 68, 68, QPen(Qt::NoPen), QBrush(QColor(255, 140, 40, 170)));
    destello.dibujo->setZValue(6);
    destello.dibujo->setPos(gx, gy);
    destellos.push_back(destello);

    for (Actor &actor : convoy) {
        if (!actor.vivo) continue;
        const float dx = actor.x - gx;
        const float dy = (actor.y - actor.alto * 0.45f) - gy;
        if (dx * dx + dy * dy < 120.f * 120.f) {
            actor.vida -= 2;
            if (actor.vida <= 0) {
                actor.vivo = false;
                actor.anim = 0.f;
            }
        }
    }
    if (qAbs(x - gx) < 90.f && qAbs((y - 60.f) - gy) < 90.f) herirJugador();
}

void EscenaEmboscada::herirJugador() {
    if (!vivo || invulnerable > 0.f || pose == Pose::Caido) return;
    --vidas;
    invulnerable = 0.7f;
    pose = Pose::Herido;
    poseTimer = 0.35f;
    anim = 0.f;
    if (vidas <= 0) {
        vivo = false;
        pose = Pose::Caido;
    }
}

bool EscenaEmboscada::golpeaActor(const Actor &actor, float bx, float by) const {
    const float medio = actor.ancho;
    return bx > actor.x - medio && bx < actor.x + medio
           && by > actor.y - actor.alto && by < actor.y - 8.f;
}

bool EscenaEmboscada::golpeaJugador(float bx, float by) const {
    return bx > x - 30.f && bx < x + 30.f && by > y - 120.f && by < y - 12.f;
}

void EscenaEmboscada::avanzar(float dt) {
    tiempo += dt;
    if (furiaTimer > 0.f) furiaTimer -= dt;
    if (recargaDisparo > 0.f) recargaDisparo -= dt;
    if (poseTimer > 0.f) poseTimer -= dt;
    if (invulnerable > 0.f) invulnerable -= dt;

    //El puente sube y baja con un seno. El personaje, si está encima, va con él.
    const float osc = qSin(tiempo * 1.6f) * 8.f;
    ponerPuente(osc);

    if (vivo && pose != Pose::Caido) {
        float vel = kVel;
        if (furiaTimer > 0.f) vel = 250.f;
        if (pose == Pose::Impulso) vel = 460.f;

        if (teclas.contains(Qt::Key_A) || teclas.contains(Qt::Key_Left)) {
            x -= vel * dt;
            mira = -1;
        }
        if (teclas.contains(Qt::Key_D) || teclas.contains(Qt::Key_Right)) {
            x += vel * dt;
            mira = 1;
        }
        x = qBound(70.f, x, 1600.f);

        const float suelo = sueloEn(x, osc);
        if (enAire) {
            vy += kGravedad * dt;
            y += vy * dt;
            if (y >= suelo && vy > 0.f) {
                y = suelo;
                vy = 0.f;
                enAire = false;
            }
            pose = Pose::Salta;
        } else {
            y = suelo;
        }

        const bool seMueve = teclas.contains(Qt::Key_A) || teclas.contains(Qt::Key_Left)
                             || teclas.contains(Qt::Key_D) || teclas.contains(Qt::Key_Right);

        if (poseTimer <= 0.f && !enAire) {
            pose = seMueve ? Pose::Camina : Pose::Quieto;
        }
        if (y > 860.f) {
            vivo = false;
            pose = Pose::Caido;
            y = 860.f;
        }
    }

    anim += dt;
    ponerMapache();
    avanzarConvoy(dt, osc);

    for (int i = balas.size() - 1; i >= 0; --i) {
        Bala &b = balas[i];
        b.edad += dt;
        bool quitar = false;
        if (b.granada) {
            //Parabola de la granada. El eje y de la pantalla crece hacia abajo.
            constexpr float elev = 0.62f;
            constexpr float v0 = 520.f;
            constexpr float g = 980.f;
            const float dir = b.vx >= 0.f ? 1.f : -1.f;
            b.x = b.x0 + dir * v0 * qCos(elev) * b.edad;
            b.y = b.y0 - v0 * qSin(elev) * b.edad + 0.5f * g * b.edad * b.edad;
            const float suelo = sueloEn(b.x, osc);
            if (b.edad > 0.28f && b.y >= suelo) {
                explotar(b.x, suelo);
                quitar = true;
            }
        } else {
            b.x += b.vx * dt;
            b.y += b.vy * dt;
            if (b.delJugador) {
                for (Actor &actor : convoy) {
                    if (!actor.vivo || !golpeaActor(actor, b.x, b.y)) continue;
                    actor.vida -= b.dano;
                    if (actor.vida <= 0) {
                        actor.vivo = false;
                        actor.anim = 0.f;
                    }
                    quitar = true;
                    break;
                }
            } else if (golpeaJugador(b.x, b.y)) {
                herirJugador();
                quitar = true;
            }
        }
        if (!quitar && (b.x < -80.f || b.x > kAnchoFondo + 80.f || b.y > kAltoFondo + 40.f || b.edad > 4.f)) {
            quitar = true;
        }
        if (quitar) {
            delete b.dibujo;
            balas.removeAt(i);
            continue;
        }
        if (b.dibujo) b.dibujo->setPos(b.x, b.y);
    }

    for (int i = destellos.size() - 1; i >= 0; --i) {
        destellos[i].vida -= dt;
        if (destellos[i].vida <= 0.f) {
            delete destellos[i].dibujo;
            destellos.removeAt(i);
        }
    }
}
