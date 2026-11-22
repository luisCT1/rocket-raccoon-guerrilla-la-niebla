#include "Sonido.h"
#include "Eventos.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QSoundEffect>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioOutput>
#endif

#include <algorithm>

namespace {
const char *const kEfectos[] = {
    "agachar", "alarma", "caida", "canon", "casco", "clic", "correo", "cuerpo", "disparo", "enemigo",
    "explosion", "furia", "golpe", "impulso", "madera", "paso", "recogida", "relincho", "sable",
    "salto", "viento", "voz_canon", "voz_fuego", "voz_penon", "voz_rey", "voz_rinde", "voz_sable"};

constexpr int kCopias = 2;
constexpr float kVolumenEfectos = 0.8f;
constexpr float kVolumenMusica = 0.4f;
constexpr qint64 kEsperaMinima = 45;
constexpr int kPeriodoAmbiente = 9000;
}

Sonido::Sonido(QObject *padre) : QObject(padre) {
    cronometro.start();

    const QString base = QCoreApplication::applicationDirPath();
    const QStringList candidatas = {base + QStringLiteral("/sonidos"),
                                    base + QStringLiteral("/../recursos/sonidos"),
                                    base + QStringLiteral("/../../recursos/sonidos"),
                                    QDir::currentPath() + QStringLiteral("/recursos/sonidos")};
    for (const QString &candidata : candidatas) {
        if (QFileInfo::exists(candidata + QStringLiteral("/clic.wav"))) {
            carpeta = QDir::cleanPath(candidata) + QLatin1Char('/');
            break;
        }
    }
    if (carpeta.isEmpty())
        return;

    listo = true;
    cargarEfectos();

    musica = new QMediaPlayer(this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    salida = new QAudioOutput(this);
    salida->setVolume(kVolumenMusica);
    musica->setAudioOutput(salida);
    musica->setLoops(-1);
#else
    musica->setVolume(static_cast<int>(kVolumenMusica * 100.f));
    connect(musica, &QMediaPlayer::mediaStatusChanged, this, [this](QMediaPlayer::MediaStatus estado) {
        if (estado == QMediaPlayer::EndOfMedia && !musicaActual.isEmpty()) {
            musica->setPosition(0);
            musica->play();
        }
    });
#endif

    ambiente = new QTimer(this);
    ambiente->setInterval(kPeriodoAmbiente);
    connect(ambiente, &QTimer::timeout, this, &Sonido::sonarAmbiente);
}

void Sonido::cargarEfectos() {
    for (const char *nombre : kEfectos) {
        const QString clave = QString::fromLatin1(nombre);
        QVector<QSoundEffect *> copias;
        copias.reserve(kCopias);
        for (int i = 0; i < kCopias; ++i) {
            auto *efecto = new QSoundEffect(this);
            efecto->setSource(QUrl::fromLocalFile(carpeta + clave + QStringLiteral(".wav")));
            copias.append(efecto);
        }
        efectos.insert(clave, copias);
    }
}

void Sonido::ponerMusica(const QString &archivo) {
    if (!listo || !musica)
        return;
    musicaActual = archivo;
    const QUrl url = QUrl::fromLocalFile(carpeta + archivo);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    musica->setSource(url);
#else
    musica->setMedia(url);
#endif
    musica->play();
}

void Sonido::iniciarMenu() {
    nivelActual = 0;
    if (ambiente)
        ambiente->stop();
    ponerMusica(QStringLiteral("fondo.mp3"));
}

void Sonido::iniciarNivel(int numero) {
    nivelActual = numero;
    golpesRecibidos = 0;
    ponerMusica(QStringLiteral("nivel%1.mp3").arg(numero));
    if (!ambiente)
        return;
    if (numero == NIVEL_PENON) {
        ambiente->start();
        tocarLuego(QStringLiteral("voz_penon"), 1200);
        tocarLuego(QStringLiteral("viento"), 400, 0.35f);
    } else {
        ambiente->stop();
    }
}

void Sonido::detenerMusica() {
    musicaActual.clear();
    if (musica)
        musica->stop();
    if (ambiente)
        ambiente->stop();
}

void Sonido::sonarAmbiente() {
    tocar(QStringLiteral("viento"), 0.35f);
}

void Sonido::tocar(const QString &nombre, float volumen) {
    if (!listo)
        return;
    auto it = efectos.find(nombre);
    if (it == efectos.end())
        return;
    const qint64 ahora = cronometro.elapsed();
    if (ahora - ultimo.value(nombre, -1000) < kEsperaMinima)
        return;
    ultimo.insert(nombre, ahora);
    int &indice = turno[nombre];
    QSoundEffect *efecto = it.value().at(indice);
    indice = (indice + 1) % it.value().size();
    if (efecto->status() != QSoundEffect::Ready)
        return;
    efecto->setVolume(std::min(1.f, volumen * kVolumenEfectos));
    efecto->play();
}

void Sonido::tocarLuego(const QString &nombre, int retrasoMs, float volumen) {
    if (!listo)
        return;
    QTimer::singleShot(retrasoMs, this, [this, nombre, volumen] { tocar(nombre, volumen); });
}

void Sonido::ritmo(const QString &nombre, bool activo, int cadaMs) {
    if (!listo || !activo)
        return;
    const qint64 ahora = cronometro.elapsed();
    if (ahora - ritmos.value(nombre, -100000) < cadaMs)
        return;
    ritmos.insert(nombre, ahora);
    tocar(nombre, 0.6f);
}

void Sonido::procesar(const std::vector<short> &eventos) {
    if (!listo)
        return;
    for (short evento : eventos) {
        switch (evento) {
        case EV_DISPARO_ROCKET:
            tocar(QStringLiteral("disparo"));
            break;
        case EV_GRANADA:
            tocar(QStringLiteral("agachar"));
            break;
        case EV_EXPLOSION:
        case EV_BARRIL:
            tocar(QStringLiteral("explosion"));
            break;
        case EV_DASH:
            tocar(QStringLiteral("impulso"));
            break;
        case EV_SALTO:
            tocar(QStringLiteral("salto"));
            break;
        case EV_GOLPE_A_ROCKET:
            tocar(QStringLiteral("golpe"));
            if (nivelActual == NIVEL_PENON && ++golpesRecibidos % 3 == 0)
                tocarLuego(QStringLiteral("voz_rinde"), 250);
            break;
        case EV_CAIDA:
            tocar(QStringLiteral("caida"));
            break;
        case EV_FURIA_ACTIVADA:
            tocar(QStringLiteral("furia"));
            break;
        case EV_ENEMIGO_CAE:
            tocar(QStringLiteral("cuerpo"));
            break;
        case EV_DISPARO_ENEMIGO:
            tocar(QStringLiteral("enemigo"), 0.8f);
            break;
        case EV_TAREA_COMPLETADA:
            tocar(nivelActual == NIVEL_EMBOSCADA ? QStringLiteral("madera") : QStringLiteral("recogida"));
            break;
        case EV_ALARMA:
        case EV_REFUERZOS:
            tocar(QStringLiteral("alarma"));
            break;
        case EV_CANON_DISPARA:
            tocar(QStringLiteral("canon"));
            if (nivelActual == NIVEL_PENON)
                tocar(QStringLiteral("voz_fuego"));
            break;
        case EV_CORREO_HUYE:
            tocar(QStringLiteral("correo"));
            tocar(QStringLiteral("relincho"));
            break;
        case EV_ATAQUE_GIRATORIO:
            tocar(QStringLiteral("sable"));
            tocar(QStringLiteral("voz_sable"));
            break;
        case EV_TROMBA:
            tocar(QStringLiteral("viento"));
            break;
        default:
            break;
        }
    }
}
