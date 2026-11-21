#include "EscenaAsalto.h"

#include "Enemigo.h"
#include "Estructura.h"
#include "Eventos.h"
#include "Juego.h"
#include "NivelAsalto.h"
#include "Proyectil.h"
#include "Rocket.h"
#include "Sonido.h"

#include <QFont>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsRectItem>
#include <QGraphicsSimpleTextItem>
#include <QPainter>
#include <QTransform>

#include <algorithm>
#include <cmath>

namespace {
const float kVistaAncho = 1280.f;
const float kVistaAlto = 720.f;

QPixmap ajustar(const QImage &img) {
    int x0 = img.width();
    int y0 = img.height();
    int x1 = -1;
    int y1 = -1;
    for (int yy = 0; yy < img.height(); ++yy) {
        for (int xx = 0; xx < img.width(); ++xx) {
            if (qAlpha(img.pixel(xx, yy)) < 28)
                continue;
            x0 = std::min(x0, xx);
            y0 = std::min(y0, yy);
            x1 = std::max(x1, xx);
            y1 = std::max(y1, yy);
        }
    }
    if (x1 < x0)
        return {};
    return QPixmap::fromImage(img.copy(x0, y0, x1 - x0 + 1, y1 - y0 + 1));
}

QVector<QPixmap> cuadros(const QPixmap &hoja, std::initializer_list<QRect> zonas) {
    QVector<QPixmap> salida;
    if (hoja.isNull())
        return salida;
    for (const QRect &zona : zonas) {
        const QRect recorte = zona.intersected(hoja.rect());
        if (recorte.isEmpty())
            continue;
        const QPixmap cuadro = ajustar(hoja.copy(recorte).toImage().convertToFormat(QImage::Format_ARGB32));
        if (!cuadro.isNull())
            salida.push_back(cuadro);
    }
    return salida;
}

QPixmap balaBrillante(const QColor &color, int diametro, bool furia) {
    const int lado = diametro + 10;
    QPixmap mapa(lado, lado);
    mapa.fill(Qt::transparent);
    QPainter pintor(&mapa);
    pintor.setRenderHint(QPainter::Antialiasing);
    pintor.setPen(Qt::NoPen);
    pintor.setBrush(furia ? QColor(255, 70, 16, 120) : QColor(color.red(), color.green(), color.blue(), 90));
    pintor.drawEllipse(0, 0, lado - 1, lado - 1);
    pintor.setPen(QPen(furia ? QColor(70, 8, 4) : QColor(36, 18, 6), 2));
    pintor.setBrush(color);
    pintor.drawEllipse(5, 5, diametro - 1, diametro - 1);
    pintor.setPen(Qt::NoPen);
    pintor.setBrush(furia ? QColor(255, 214, 120) : QColor(255, 248, 214));
    const int nucleo = qMax(6, diametro / 3);
    pintor.drawEllipse(5 + (diametro - nucleo) / 2, 5 + (diametro - nucleo) / 2, nucleo, nucleo);
    return mapa;
}

QPixmap escalarFijo(const QPixmap &cuadro, float alto, float altoFuente) {
    if (cuadro.isNull())
        return {};
    const float ref = altoFuente > 1.f ? altoFuente : float(qMax(1, cuadro.height()));
    const float factor = alto / ref;
    return cuadro.scaledToHeight(qMax(1, qRound(cuadro.height() * factor)), Qt::SmoothTransformation);
}

QPixmap pintarCaja(const QColor &madera, bool roto) {
    QPixmap px(128, 128);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(48, 28, 14), 4));
    p.setBrush(madera);
    p.drawRoundedRect(10, 22, 108, 88, 8, 8);
    p.setBrush(roto ? QColor(70, 48, 28) : QColor(214, 176, 78));
    p.drawEllipse(QRectF(42, 40, 44, 44));
    if (roto) {
        p.setPen(QPen(QColor(40, 22, 12), 3));
        p.drawLine(18, 30, 70, 100);
        p.drawLine(108, 28, 40, 108);
    }
    return px;
}

QPixmap pintarBandera(bool caida) {
    QPixmap px(90, 130);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(72, 48, 24), 5));
    p.drawLine(24, 12, 24, 120);
    p.setPen(Qt::NoPen);
    p.setBrush(caida ? QColor(120, 120, 120) : QColor(214, 48, 42));
    if (caida)
        p.drawRoundedRect(18, 96, 62, 22, 4, 4);
    else {
        QPolygonF tela;
        tela << QPointF(26, 16) << QPointF(82, 34) << QPointF(26, 54);
        p.drawPolygon(tela);
    }
    return px;
}

QPixmap pintarCanon(bool roto) {
    QPixmap px(120, 80);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(28, 28, 30), 3));
    p.setBrush(roto ? QColor(70, 64, 58) : QColor(42, 46, 52));
    p.drawRoundedRect(8, 28, 46, 36, 6, 6);
    p.drawRoundedRect(40, 36, 68, 16, 4, 4);
    p.setBrush(QColor(92, 78, 48));
    p.drawEllipse(14, 52, 18, 18);
    p.drawEllipse(34, 52, 18, 18);
    return px;
}

QPixmap pintarTorre() {
    QPixmap px(96, 96);
    px.fill(Qt::transparent);
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(QColor(70, 64, 52), 3));
    p.setBrush(QColor(128, 118, 96));
    p.drawEllipse(8, 8, 80, 80);
    p.setBrush(QColor(86, 78, 62));
    p.drawEllipse(28, 28, 40, 40);
    return px;
}
}

EscenaAsalto::EscenaAsalto() {
    setSceneRect(0, 0, 1600, 1200);
    setBackgroundBrush(QColor(78, 112, 68));
    sonidos = new Sonido(this);
    cargarSprites();
    juego = new Juego();
    juego->iniciarNivel(NIVEL_ASALTO, DIF_NORMAL);
    nivel = dynamic_cast<NivelAsalto *>(juego->nivelActivo());
    if (nivel)
        setSceneRect(0, 0, nivel->getAnchoMundo(), nivel->getAltoMundo());
    armarPatio();
}

EscenaAsalto::~EscenaAsalto() {
    delete sonidos;
    delete juego;
}

void EscenaAsalto::cargarSprites() {
    const QPixmap rocket(QStringLiteral(":/nivel1/sprites_rocket.png"));
    caminar = cuadros(rocket, {QRect(18, 36, 153, 139), QRect(188, 36, 145, 139), QRect(353, 36, 147, 139),
                               QRect(521, 36, 144, 139), QRect(691, 36, 134, 139), QRect(847, 36, 147, 139)});
    disparar = cuadros(rocket, {QRect(20, 406, 162, 139), QRect(189, 406, 143, 139), QRect(356, 406, 167, 139),
                                QRect(527, 406, 171, 139), QRect(699, 406, 135, 139), QRect(852, 406, 148, 139)});
    lanzar = cuadros(rocket, {QRect(368, 595, 167, 141), QRect(542, 595, 121, 141), QRect(699, 595, 131, 141)});
    impulso = cuadros(rocket, {QRect(14, 783, 157, 150), QRect(182, 783, 161, 150),
                               QRect(355, 783, 149, 150), QRect(527, 783, 144, 150)});
    furia = cuadros(rocket, {QRect(15, 1340, 164, 155), QRect(198, 1340, 140, 155), QRect(522, 1340, 140, 155)});
    rocketMuerte = cuadros(rocket, {QRect(684, 960, 147, 151), QRect(851, 960, 154, 151)});

    const QPixmap soldados(QStringLiteral(":/nivel1/sprites_soldados.png"));
    soldadoCamina = cuadros(soldados, {QRect(62, 39, 132, 177), QRect(320, 39, 126, 177), QRect(574, 39, 131, 177),
                                       QRect(828, 39, 135, 177), QRect(1083, 39, 137, 177), QRect(1334, 39, 148, 177)});
    soldadoDispara = cuadros(soldados, {QRect(58, 1073, 140, 157), QRect(268, 1073, 232, 157), QRect(594, 1073, 91, 157),
                                        QRect(858, 1073, 76, 157), QRect(1099, 1073, 106, 157), QRect(1360, 1073, 96, 157)});

    polvorin = pintarCaja(QColor(150, 104, 54), false);
    polvorinRoto = pintarCaja(QColor(96, 72, 48), true);
    bandera = pintarBandera(false);
    banderaCaida = pintarBandera(true);
    canon = pintarCanon(false);
    canonRoto = pintarCanon(true);
    torre = pintarTorre();

    const QPixmap hoja(QStringLiteral(":/nivel1/sprint_vidas_municion.png"));
    if (!hoja.isNull())
        prepararMarcador(hoja.toImage().convertToFormat(QImage::Format_ARGB32), piezasHud);
}

void EscenaAsalto::armarPatio() {
    auto *pasto = addRect(0, 0, 1600, 1200, Qt::NoPen, QColor(74, 108, 62));
    pasto->setZValue(0);
    auto *patio = addRect(360, 140, 980, 820, Qt::NoPen, QColor(112, 108, 86));
    patio->setZValue(0.1);
    auto *sendero = addRect(40, 900, 420, 220, Qt::NoPen, QColor(138, 112, 72));
    sendero->setZValue(0.15);

    const QPixmap vegetacion(QStringLiteral(":/nivel3/vegetacion.png"));
    const QVector<QPixmap> copas = cuadros(vegetacion, {
        QRect(9, 13, 373, 486), QRect(422, 13, 340, 486), QRect(774, 13, 367, 486)});
    const QPointF puestos[] = {{80, 80}, {1380, 60}, {40, 620}, {1460, 480}, {200, 980}, {1320, 980}};
    for (int i = 0; i < 6 && !copas.isEmpty(); ++i) {
        const QPixmap arbol = copas.at(i % copas.size()).scaledToHeight(210, Qt::SmoothTransformation);
        auto *item = addPixmap(arbol);
        item->setPos(puestos[i]);
        item->setZValue(0.8);
        item->setOpacity(0.92);
    }

    salida = addEllipse(80, 980, 140, 140, QPen(QColor(226, 177, 90), 4), QBrush(QColor(226, 177, 90, 50)));
    salida->setZValue(1.1);
    salida->hide();

    reloj = addSimpleText(QString());
    reloj->setFont(QFont(QStringLiteral("Georgia"), 22, QFont::Bold));
    reloj->setBrush(QColor(255, 228, 150));
    reloj->setZValue(41);

    tareas = addSimpleText(QString());
    tareas->setFont(QFont(QStringLiteral("Georgia"), 14, QFont::Bold));
    tareas->setBrush(QColor(246, 236, 210));
    tareas->setZValue(41);
}

void EscenaAsalto::pulsar(int tecla) {
    teclas.insert(tecla);
    if (!nivel || nivel->estaTerminado() || !nivel->jugador())
        return;
    if (tecla == Qt::Key_Shift)
        nivel->jugadorDash();
    if (tecla == Qt::Key_F)
        nivel->jugadorActivaFuria();
    if (tecla == Qt::Key_G) {
        Rocket *rocket = nivel->jugador();
        const float ax = hayApunte ? punteroX : rocket->getCentroX() + 180.f;
        const float ay = hayApunte ? punteroY : rocket->getCentroY();
        nivel->jugadorLanzaGranada(ax, ay);
        granadaMuestra = 0.45f;
    }
}

void EscenaAsalto::soltar(int tecla) {
    teclas.remove(tecla);
}

void EscenaAsalto::mantener(int tecla) {
    teclas.insert(tecla);
}

void EscenaAsalto::apuntar(float x, float y) {
    punteroX = x;
    punteroY = y;
    hayApunte = true;
}

void EscenaAsalto::avanzar(float dt) {
    if (!nivel)
        return;
    tiempo += dt;
    if (granadaMuestra > 0.f)
        granadaMuestra = std::max(0.f, granadaMuestra - dt);

    Rocket *rocket = nivel->jugador();
    if (rocket && !nivel->estaTerminado()) {
        if (teclas.contains(Qt::Key_A) || teclas.contains(Qt::Key_Left))
            nivel->jugadorMueve('L');
        if (teclas.contains(Qt::Key_D) || teclas.contains(Qt::Key_Right))
            nivel->jugadorMueve('R');
        if (teclas.contains(Qt::Key_W) || teclas.contains(Qt::Key_Up))
            nivel->jugadorMueve('U');
        if (teclas.contains(Qt::Key_S) || teclas.contains(Qt::Key_Down))
            nivel->jugadorMueve('D');
        if (teclas.contains(Qt::Key_Space)) {
            const float ax = hayApunte ? punteroX : rocket->getCentroX() + (rocket->getDireccion() == 'L' ? -360.f : 360.f);
            const float ay = hayApunte ? punteroY : rocket->getCentroY();
            nivel->jugadorDispara(ax, ay);
        }
    }

    juego->actualizar(dt);

    rocket = nivel->jugador();
    if (rocket) {
        const float ancho = nivel->getAnchoMundo();
        const float alto = nivel->getAltoMundo();
        const float deseoX = std::clamp(rocket->getCentroX() - kVistaAncho * 0.45f, 0.f, std::max(0.f, ancho - kVistaAncho));
        const float deseoY = std::clamp(rocket->getCentroY() - kVistaAlto * 0.55f, 0.f, std::max(0.f, alto - kVistaAlto));
        const float paso = std::min(1.f, dt * 5.f);
        camX += (deseoX - camX) * paso;
        camY += (deseoY - camY) * paso;
    }

    sincronizar();
    dibujarHud();

    const int segundos = nivel ? int(std::ceil(nivel->getTiempoRestante())) : 0;
    if (reloj) {
        reloj->setText(QStringLiteral("%1:%2")
                           .arg(segundos / 60)
                           .arg(segundos % 60, 2, 10, QChar('0')));
        reloj->setPos(camX + kVistaAncho - 130.f, camY + 12.f);
    }
    if (tareas && nivel) {
        const auto marca = [](bool hecho, const char *nombre) {
            return QString(hecho ? QStringLiteral("✓ ") : QStringLiteral("· ")) + QString::fromUtf8(nombre);
        };
        tareas->setText(marca(nivel->estaPolvorinVolado(), "Polvorín") + QStringLiteral("    ")
                        + marca(nivel->estaBanderaTumbada(), "Bandera") + QStringLiteral("    ")
                        + marca(nivel->estaCanonInutilizado(), "Cañón"));
        tareas->setPos(camX + kVistaAncho - 430.f, camY + 52.f);
    }
    if (salida)
        salida->setVisible(nivel->tareasCompletas());
}

QRectF EscenaAsalto::rectCamara() const {
    return QRectF(camX, camY, kVistaAncho, kVistaAlto);
}

QString EscenaAsalto::textoHud() const {
    return {};
}

QString EscenaAsalto::mensajeFinal() const {
    if (!nivel || !nivel->estaTerminado())
        return {};
    if (nivel->fueGanado())
        return QStringLiteral("Misión cumplida\nEl patio es de la guerrilla.");
    if (nivel->getTiempoRestante() <= 0.f)
        return QStringLiteral("Misión fallida\nSe acabó el tiempo del asalto.");
    return QStringLiteral("Misión fallida\nTe derrotaron en el patio.");
}

bool EscenaAsalto::enFuria() const {
    Rocket *rocket = nivel ? nivel->jugador() : nullptr;
    return rocket && rocket->estaFurioso();
}

bool EscenaAsalto::gano() const {
    return nivel && nivel->fueGanado();
}

bool EscenaAsalto::acabo() const {
    return nivel && nivel->estaTerminado();
}

QPixmap EscenaAsalto::cuadroDe(Entidad *entidad) const {
    const QVector<QPixmap> *lista = nullptr;
    int indice = 0;
    if (entidad->clase() == CLASE_ROCKET) {
        auto *rocket = static_cast<Rocket *>(entidad);
        if (rocket->getVidas() <= 0 && !rocketMuerte.isEmpty())
            lista = &rocketMuerte;
        else if (rocket->estadoVisual() == 3 && !impulso.isEmpty())
            lista = &impulso;
        else if (rocket->estaFurioso() && !furia.isEmpty())
            lista = &furia;
        else if (granadaMuestra > 0.f && !lanzar.isEmpty())
            lista = &lanzar;
        else if (teclas.contains(Qt::Key_Space) && !disparar.isEmpty())
            lista = &disparar;
        else
            lista = &caminar;
        if (rocket->getVidas() <= 0)
            indice = lista->isEmpty() ? 0 : lista->size() - 1;
        else if (rocket->estadoVisual() == 0 && lista == &caminar)
            indice = 0;
        else
            indice = int(tiempo * 8.f);
    } else if (entidad->clase() == CLASE_ENEMIGO) {
        auto *enemigo = static_cast<Enemigo *>(entidad);
        lista = enemigo->llevaArma() && !soldadoDispara.isEmpty() ? &soldadoDispara : &soldadoCamina;
        indice = enemigo->llevaArma() ? int(tiempo * 8.f) : 0;
    }
    if (!lista || lista->isEmpty())
        return {};
    if (indice < 0)
        indice = 0;
    QPixmap cuadro = lista->at(indice % lista->size());
    char dir = 'R';
    if (auto *rocket = dynamic_cast<Rocket *>(entidad))
        dir = rocket->getDireccion();
    else if (auto *enemigo = dynamic_cast<Enemigo *>(entidad))
        dir = enemigo->getDireccion();
    if (dir == 'L')
        cuadro = cuadro.transformed(QTransform().scale(-1, 1));
    return cuadro;
}

void EscenaAsalto::colocar(Entidad *entidad, QGraphicsPixmapItem *sprite) {
    if (entidad->clase() == CLASE_ESTRUCTURA) {
        auto *pieza = static_cast<Estructura *>(entidad);
        QPixmap cuadro;
        float alto = 90.f;
        switch (pieza->getTipo()) {
        case EST_POLVORIN:
            cuadro = pieza->estaDestruida() ? polvorinRoto : polvorin;
            alto = 110.f;
            break;
        case EST_BANDERA:
            cuadro = pieza->estaDestruida() ? banderaCaida : bandera;
            alto = 120.f;
            break;
        case EST_CANON:
            cuadro = pieza->estaDestruida() ? canonRoto : canon;
            alto = 78.f;
            break;
        case EST_TORRE:
            cuadro = torre;
            alto = 96.f;
            break;
        default:
            break;
        }
        if (cuadro.isNull())
            return;
        const QPixmap puesto = cuadro.scaledToHeight(qRound(alto), Qt::SmoothTransformation);
        sprite->setPixmap(puesto);
        sprite->setPos(entidad->getCentroX() - puesto.width() / 2.f, entidad->getCentroY() - puesto.height() / 2.f);
        sprite->setZValue(2.2);
        return;
    }

    QPixmap cuadro = cuadroDe(entidad);
    if (cuadro.isNull())
        return;
    float alto = 78.f;
    float fuente = 0.f;
    if (entidad->clase() == CLASE_ROCKET) {
        alto = 86.f;
        fuente = caminar.isEmpty() ? 0.f : float(caminar.first().height());
    } else if (entidad->clase() == CLASE_ENEMIGO) {
        alto = 80.f;
        fuente = soldadoCamina.isEmpty() ? 0.f : float(soldadoCamina.first().height());
    }
    const QPixmap puesto = escalarFijo(cuadro, alto, fuente);
    sprite->setPixmap(puesto);
    sprite->setPos(entidad->getCentroX() - puesto.width() / 2.f, entidad->getCentroY() - puesto.height() * 0.72f);
    sprite->setZValue(3.f + entidad->getY() * 0.001f);
    if (auto *rocket = dynamic_cast<Rocket *>(entidad))
        sprite->setOpacity(!rocket->esInvulnerable() || std::fmod(tiempo, 0.24f) < 0.12f ? 1.0 : 0.35);
    else
        sprite->setOpacity(1.0);
}

void EscenaAsalto::sincronizar() {
    if (!nivel)
        return;
    const std::vector<Entidad *> &lista = nivel->obtenerEntidades();
    QSet<Entidad *> vivas;
    for (Entidad *entidad : lista) {
        vivas.insert(entidad);
        QGraphicsPixmapItem *sprite = piezas.value(entidad, nullptr);
        if (!sprite) {
            sprite = addPixmap(QPixmap());
            if (entidad->clase() == CLASE_PROYECTIL) {
                auto *bala = static_cast<Proyectil *>(entidad);
                QPixmap punto;
                if (bala->esBombeado())
                    punto = balaBrillante(QColor(72, 64, 56), 40, false);
                else if (bala->esDeRocket() && bala->getDanio() > 30.f)
                    punto = balaBrillante(QColor(255, 64, 18), 26, true);
                else if (bala->esDeRocket())
                    punto = balaBrillante(QColor(255, 214, 70), 22, false);
                else
                    punto = balaBrillante(QColor(255, 48, 36), 18, false);
                sprite->setPixmap(punto);
                sprite->setZValue(4);
            }
            piezas.insert(entidad, sprite);
        }
        if (entidad->clase() == CLASE_PROYECTIL) {
            const QSize s = sprite->pixmap().size();
            sprite->setPos(entidad->getCentroX() - s.width() / 2.0, entidad->getCentroY() - s.height() / 2.0);
            continue;
        }
        colocar(entidad, sprite);
    }
    const auto llaves = piezas.keys();
    for (Entidad *entidad : llaves) {
        if (vivas.contains(entidad))
            continue;
        QGraphicsPixmapItem *sprite = piezas.take(entidad);
        removeItem(sprite);
        delete sprite;
    }
    std::vector<short> eventos;
    nivel->tomarEventos(eventos);
    if (sonidos) {
        Rocket *rocket = nivel->jugador();
        sonidos->eventos(eventos, rocket && rocket->estaFurioso());
    }
}

void EscenaAsalto::dibujarHud() {
    if (!nivel || !nivel->jugador() || piezasHud.vidas.isNull())
        return;
    Rocket *rocket = nivel->jugador();
    const QImage lienzo = componerMarcador(piezasHud, rocket->getVidas(),
        rocket->getSaludMax() > 0.f ? rocket->getSalud() / rocket->getSaludMax() : 0.f,
        rocket->getMunicion(), rocket->getGranadas(), rocket->getBarraFuria());
    if (lienzo.isNull())
        return;
    if (!marcador) {
        marcador = addPixmap(QPixmap());
        marcador->setZValue(40);
    }
    marcador->setPixmap(QPixmap::fromImage(lienzo));
    marcador->setPos(camX + 52.f, camY + 42.f);
}
