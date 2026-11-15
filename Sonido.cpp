#include "Sonido.h"
#include "Eventos.h"

#include <QUrl>

Sonido::Sonido(const QString &carpetaBase, QObject *padre)
    : QObject(padre), carpeta(carpetaBase), musica(nullptr), activo(true) {
    const char *nombres[] = {"disparo", "explosion", "golpe", "alarma", "victoria", "derrota"};
    for (const char *n : nombres) cargar(n);

    musica = new QSoundEffect(this);
    musica->setSource(QUrl::fromLocalFile(carpeta + "musica.wav"));
    musica->setLoopCount(QSoundEffect::Infinite);
    musica->setVolume(0.35f);
}

QSoundEffect *Sonido::cargar(const QString &nombre) {
    auto it = efectos.constFind(nombre);
    if (it != efectos.constEnd()) return it.value();
    QSoundEffect *e = new QSoundEffect(this);
    e->setSource(QUrl::fromLocalFile(carpeta + nombre + ".wav"));
    e->setVolume(0.8f);
    efectos.insert(nombre, e);
    return e;
}

void Sonido::tocar(const QString &nombre) {
    if (!activo) return;
    QSoundEffect *e = cargar(nombre);
    if (e->status() == QSoundEffect::Ready) e->play();
}

void Sonido::iniciarMusica() {
    if (activo && musica->status() == QSoundEffect::Ready && !musica->isPlaying()) musica->play();
}

void Sonido::detenerMusica() { musica->stop(); }

void Sonido::setActivo(bool a) {
    activo = a;
    if (!a) musica->stop();
    else iniciarMusica();
}

void Sonido::procesarEventos(const std::vector<short> &eventos) {
    for (short ev : eventos) {
        switch (ev) {
        case EV_DISPARO_ROCKET:
        case EV_DISPARO_ENEMIGO:
        case EV_CANON_DISPARA: tocar("disparo"); break;
        case EV_EXPLOSION:
        case EV_GRANADA:
        case EV_TROMBA: tocar("explosion"); break;
        case EV_GOLPE_A_ROCKET:
        case EV_CAIDA: tocar("golpe"); break;
        case EV_ALARMA: tocar("alarma"); break;
        case EV_VICTORIA: tocar("victoria"); break;
        case EV_DERROTA: tocar("derrota"); break;
        default: break;
        }
    }
}
