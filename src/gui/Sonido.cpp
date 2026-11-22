#include "Sonido.h"
#include "Eventos.h"

#include <QAudioOutput>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMediaPlayer>
#include <QRandomGenerator>
#include <QSoundEffect>
#include <QTimer>
#include <QUrl>

namespace {
const QString kAlimana = QStringLiteral("N3_voz_alimana");
const QString kImposible = QStringLiteral("N3_voz_imposible");
const QString kSalidos = QStringLiteral("rocket_salidos");
const QString kMision = QStringLiteral("rocket_mision_cumplida");
const QString kRespeto = QStringLiteral("rocket_ya_respetame");
}

Sonido::Sonido(QObject *padre_, short nivel_) : padre(padre_), nivel(nivel_) {
    poner("disparo", "disparo.wav", 0.76f);
    poner("enemigo", "enemigo.wav", 0.36f);
    poner("explosion", "explosion.wav", 0.62f);
    poner("furia", "furia.wav", 0.68f);
    poner("granada", "impulso.wav", 0.28f);
    poner("impulso", "impulso.wav", 0.40f);
    poner("salto", "salto.wav", 0.26f);
    poner("golpe", "golpe.wav", 0.56f);
    poner("caida", "caida.wav", 0.48f);
    poner("cuerpo", "cuerpo.wav", 0.36f);
    poner("correo", "correo.wav", 0.42f);
    poner("recogida", "recogida.wav", 0.46f);
    poner("recoVida", "reco_vida.wav", 0.72f);
    poner("recoMunicion", "reco_municion.wav", 0.68f);
    poner("recoGranada", "reco_granada.wav", 0.70f);
    poner("recoFuria", "reco_furia.wav", 0.74f);
    poner("alarma", "alarma.wav", 0.50f);
    poner("canon", "canon.wav", 0.38f);
    poner("madera", "madera.wav", 0.38f);
    poner("paso", "paso.wav", 0.28f);
    poner("agachar", "agachar.wav", 0.32f);
    poner("casco", "casco.wav", 0.74f);
    poner("marcha", "paso.wav", 0.18f);
    poner("carreta", "correo.wav", 0.24f);
    poner("aterrizar", "caida.wav", 0.30f);
    poner("sable", "sable.wav", 0.66f);
    poner("viento", "viento.wav", 0.58f);
    poner("relincho", "relincho.wav", 0.72f);

    reloj.start();
    salidaRocket = new QAudioOutput(padre);
    salidaRocket->setVolume(1.f);
    vozRocket = new QMediaPlayer(padre);
    vozRocket->setAudioOutput(salidaRocket);
    QMediaPlayer *reproductor = vozRocket;
    QObject::connect(vozRocket, &QMediaPlayer::mediaStatusChanged, padre,
                     [this, reproductor](QMediaPlayer::MediaStatus estado) {
                         if (estado == QMediaPlayer::LoadedMedia
                             && reproductor->playbackState() != QMediaPlayer::PlayingState)
                             reproductor->play();
                         if (estado == QMediaPlayer::EndOfMedia)
                             cerrarSalidos();
                     });
    QObject::connect(vozRocket, &QMediaPlayer::errorOccurred, padre,
                     [this](QMediaPlayer::Error error, const QString &) {
                         if (error == QMediaPlayer::NoError)
                             return;
                         qWarning("Sonido: falló la voz de Rocket");
                         cerrarSalidos();
                     });
    salidaCapitan = new QAudioOutput(padre);
    salidaCapitan->setVolume(1.f);
    vozCapitan = new QMediaPlayer(padre);
    vozCapitan->setAudioOutput(salidaCapitan);
    QMediaPlayer *capitan = vozCapitan;
    QObject::connect(vozCapitan, &QMediaPlayer::mediaStatusChanged, padre,
                     [capitan](QMediaPlayer::MediaStatus estado) {
                         if (estado == QMediaPlayer::LoadedMedia
                             && capitan->playbackState() != QMediaPlayer::PlayingState)
                             capitan->play();
                     });
}

Sonido::~Sonido() {
    if (vozRocket)
        QObject::disconnect(vozRocket, nullptr, nullptr, nullptr);
    if (vozCapitan)
        QObject::disconnect(vozCapitan, nullptr, nullptr, nullptr);
    for (auto it = vocesCapitan.cbegin(); it != vocesCapitan.cend(); ++it) {
        if (*it)
            QObject::disconnect(*it, nullptr, nullptr, nullptr);
    }
}

bool Sonido::esFrecuente(const QString &nombre) const {
    return nombre == QLatin1String("rocket_tomen_esto")
        || nombre == QLatin1String("rocket_eso_dolio")
        || nombre == QLatin1String("rocket_hay_voy")
        || nombre == QLatin1String("rocket_acabaron_las_balas");
}

bool Sonido::play(const QString &nombre, bool prioridad) {
    if (nombre.isEmpty())
        return false;
    const bool rocket = nombre.startsWith(QLatin1String("rocket_"));
    const bool capitan = nombre.contains(QLatin1String("_voz_"));
    if (!rocket && !capitan) {
        qWarning("Sonido: no hay canal para %s", qPrintable(nombre));
        return false;
    }

    const qint64 ahora = reloj.elapsed();
    if (!prioridad) {
        const qint64 previo = rocket ? ultimoRocket : ultimoCapitan;
        if (ahora - previo < 2000)
            return false;
        if (!rocket && nombre == kAlimana && ahora - ultimoAlimana < 10000)
            return false;
        if (rocket && esFrecuente(nombre)) {
            if (ahora - ultimoFrecuente < 5000)
                return false;
            if (QRandomGenerator::global()->bounded(3) != 0)
                return false;
        }
    }

    const bool sono = rocket ? reproducirRocket(nombre) : reproducirCapitan(nombre, prioridad);
    if (!sono)
        return false;

    if (rocket) {
        ultimoRocket = ahora;
        if (esFrecuente(nombre))
            ultimoFrecuente = ahora;
    } else {
        ultimoCapitan = ahora;
        if (nombre == kAlimana)
            ultimoAlimana = ahora;
    }

    if (nombre == kImposible && !salidosPedido) {
        salidosPedido = true;
        QTimer::singleShot(1500, padre, [this]() {
            if (!play(kSalidos, true)) {
                salidosTermino = true;
                if (misionPendiente) {
                    misionPendiente = false;
                    play(kMision, true);
                }
            }
        });
    }
    return true;
}

void Sonido::poner(const char *clave, const char *archivo, float volumen) {
    auto *efecto = new QSoundEffect(padre);
    efecto->setSource(QUrl(QStringLiteral("qrc:/sonidos/") + QString::fromLatin1(archivo)));
    efecto->setVolume(volumen);
    clips.insert(QString::fromLatin1(clave), efecto);
}

QString Sonido::extraer(const QString &archivo) {
    if (rutasRocket.contains(archivo))
        return rutasRocket.value(archivo);
    QFile origen(QStringLiteral(":/sonidos/") + archivo);
    if (!origen.open(QIODevice::ReadOnly)) {
        if (!avisados.contains(archivo)) {
            avisados.insert(archivo);
            qWarning("Sonido: no se pudo abrir %s", qPrintable(archivo));
        }
        rutasRocket.insert(archivo, QString());
        return {};
    }
    const QString ruta = QDir::temp().filePath(QStringLiteral("niebla-") + archivo);
    QFile destino(ruta);
    if (!destino.open(QIODevice::WriteOnly)) {
        qWarning("Sonido: no se pudo preparar %s", qPrintable(archivo));
        rutasRocket.insert(archivo, QString());
        return {};
    }
    destino.write(origen.readAll());
    rutasRocket.insert(archivo, ruta);
    return ruta;
}

bool Sonido::esMp3(const QString &recurso) const {
    QFile origen(recurso);
    if (!origen.open(QIODevice::ReadOnly))
        return false;
    char marca[3] = {};
    if (origen.read(marca, 3) < 3)
        return false;
    return marca[0] == 'I' && marca[1] == 'D' && marca[2] == '3';
}

QString Sonido::extraerMp3Disfrazado(const QString &nombre) {
    const QString clave = nombre + QStringLiteral(".vozmp3");
    if (rutasRocket.contains(clave))
        return rutasRocket.value(clave);
    QFile origen(QStringLiteral(":/sonidos/") + nombre + QStringLiteral(".wav"));
    if (!origen.open(QIODevice::ReadOnly)) {
        qWarning("Sonido: no se pudo abrir %s.wav", qPrintable(nombre));
        rutasRocket.insert(clave, QString());
        return {};
    }
    const QString ruta = QDir::temp().filePath(QStringLiteral("niebla-") + nombre + QStringLiteral(".mp3"));
    QFile destino(ruta);
    if (!destino.open(QIODevice::WriteOnly)) {
        rutasRocket.insert(clave, QString());
        return {};
    }
    destino.write(origen.readAll());
    rutasRocket.insert(clave, ruta);
    return ruta;
}

QSoundEffect *Sonido::efectoDe(const QString &nombre) {
    if (vocesCapitan.contains(nombre))
        return vocesCapitan.value(nombre);
    const QString archivo = nombre + QStringLiteral(".wav");
    const QString recurso = QStringLiteral(":/sonidos/") + archivo;
    if (!QFile::exists(recurso)) {
        if (!avisados.contains(archivo)) {
            avisados.insert(archivo);
            qWarning("Sonido: no está %s", qPrintable(archivo));
        }
        vocesCapitan.insert(nombre, nullptr);
        return nullptr;
    }
    auto *efecto = new QSoundEffect(padre);
    efecto->setSource(QUrl(QStringLiteral("qrc:/sonidos/") + archivo));
    efecto->setVolume(1.f);
    QObject::connect(efecto, &QSoundEffect::statusChanged, padre, [archivo, efecto]() {
        if (efecto->status() == QSoundEffect::Error)
            qWarning("Sonido: no cargó %s", qPrintable(archivo));
    });
    if (nombre == kAlimana) {
        QObject::connect(efecto, &QSoundEffect::playingChanged, padre, [this, efecto]() {
            if (efecto->isPlaying()) {
                alimanaSono = true;
                return;
            }
            if (!alimanaSono || !respetoPendiente)
                return;
            alimanaSono = false;
            respetoPendiente = false;
            QTimer::singleShot(0, padre, [this]() { play(kRespeto, true); });
        });
    }
    vocesCapitan.insert(nombre, efecto);
    return efecto;
}

bool Sonido::reproducirCapitan(const QString &nombre, bool prioridad) {
    (void)prioridad;
    const QString archivo = nombre + QStringLiteral(".wav");
    const QString recurso = QStringLiteral(":/sonidos/") + archivo;
    if (esMp3(recurso)) {
        const QString ruta = extraerMp3Disfrazado(nombre);
        if (ruta.isEmpty() || !vozCapitan)
            return false;
        alimanaSono = false;
        respetoPendiente = false;
        if (capitanSonando) {
            capitanSonando->stop();
            capitanSonando = nullptr;
        }
        vozCapitan->stop();
        vozCapitan->setPlaybackRate(1.f);
        vozCapitan->setSource(QUrl::fromLocalFile(ruta));
        vozCapitan->play();
        return true;
    }
    QSoundEffect *efecto = efectoDe(nombre);
    if (!efecto || efecto->status() == QSoundEffect::Error)
        return false;
    if (vozCapitan)
        vozCapitan->stop();
    alimanaSono = false;
    respetoPendiente = false;
    if (capitanSonando && capitanSonando != efecto)
        capitanSonando->stop();
    if (efecto->isPlaying())
        efecto->stop();
    capitanSonando = efecto;
    respetoPendiente = nombre == kAlimana;
    efecto->play();
    return true;
}

bool Sonido::reproducirRocket(const QString &nombre) {
    const QString ruta = extraer(nombre + QStringLiteral(".mp3"));
    if (ruta.isEmpty() || !vozRocket)
        return false;
    rocketNombre = nombre;
    cambiandoRocket = true;
    vozRocket->stop();
    vozRocket->setPlaybackRate(1.f);
    vozRocket->setSource(QUrl::fromLocalFile(ruta));
    cambiandoRocket = false;
    vozRocket->play();
    return true;
}

void Sonido::cerrarSalidos() {
    if (cambiandoRocket || salidosTermino || rocketNombre != kSalidos)
        return;
    salidosTermino = true;
    if (!misionPendiente)
        return;
    misionPendiente = false;
    QTimer::singleShot(0, padre, [this]() { play(kMision, true); });
}

void Sonido::tocar(const char *clave) {
    QSoundEffect *efecto = clips.value(QString::fromLatin1(clave), nullptr);
    if (!efecto || efecto->status() == QSoundEffect::Error)
        return;
    if (efecto->isPlaying())
        efecto->stop();
    efecto->play();
}

void Sonido::eventos(const std::vector<short> &lista, bool furioso) {
    for (short evento : lista) {
        switch (evento) {
        case EV_DISPARO_ROCKET: tocar(furioso ? "furia" : "disparo"); break;
        case EV_DISPARO_ENEMIGO:
            tocar("enemigo");
            if (nivel == NIVEL_EMBOSCADA)
                play(QStringLiteral("N3_voz_fuego"));
            break;
        case EV_GRANADA:
            tocar("granada");
            play(QStringLiteral("rocket_tomen_esto"));
            break;
        case EV_EXPLOSION: tocar("explosion"); break;
        case EV_DASH:
            tocar("impulso");
            play(QStringLiteral("rocket_hay_voy"));
            // TODO: N3_voz_te_he_oido.wav no está en recursos/sonidos. No se conecta.
            break;
        case EV_SALTO: tocar("salto"); break;
        case EV_GOLPE_A_ROCKET:
            tocar("golpe");
            play(QStringLiteral("rocket_eso_dolio"));
            break;
        case EV_CAIDA: tocar("caida"); break;
        case EV_FURIA_ACTIVADA:
            tocar("furia");
            play(QStringLiteral("rocket_se_armo"));
            break;
        case EV_FURIA_FIN: play(QStringLiteral("rocket_ya_me_canse")); break;
        case EV_SIN_BALAS: play(QStringLiteral("rocket_acabaron_las_balas")); break;
        case EV_VIDA_PERDIDA: play(QStringLiteral("rocket_ay_no")); break;
        case EV_ENEMIGO_CAE: tocar("cuerpo"); break;
        case EV_CORREO_HUYE:
            tocar("correo");
            // TODO: «¡Que no escape el correo!» no tiene archivo. EV_CORREO_HUYE queda sin voz.
            break;
        case EV_RECOGIDA: tocar("recogida"); break;
        case EV_RECO_VIDA: tocar("recoVida"); break;
        case EV_RECO_MUNICION: tocar("recoMunicion"); break;
        case EV_RECO_GRANADA: tocar("recoGranada"); break;
        case EV_RECO_FURIA: tocar("recoFuria"); break;
        case EV_PASO: tocar("paso"); break;
        case EV_AGACHARSE: tocar("agachar"); break;
        case EV_CASCO: tocar("casco"); break;
        case EV_MARCHA: tocar("marcha"); break;
        case EV_CARRETA: tocar("carreta"); break;
        case EV_ATERRIZAJE: tocar("aterrizar"); break;
        case EV_ALARMA: tocar("alarma"); break;
        case EV_CANON_DISPARA: tocar("canon"); tocar("explosion"); break;
        case EV_TAREA_COMPLETADA: tocar("madera"); break;
        case EV_ATAQUE_GIRATORIO:
            tocar("sable");
            if (nivel == NIVEL_PENON)
                play(QStringLiteral("N3_voz_merecido"));
            break;
        case EV_SABLE: tocar("sable"); break;
        case EV_TROMBA: tocar("viento"); tocar("sable"); break;
        case EV_VIENTO: tocar("viento"); break;
        case EV_RELINCHO: tocar("relincho"); break;
        case EV_CAMBIO_FASE_JEFE: tocar("alarma"); break;
        case EV_VOZ_EMBOSCADA: play(QStringLiteral("N1_voz_emboscada")); break;
        case EV_VOZ_CONVOY: play(QStringLiteral("N1_voz_defender_convoy")); break;
        case EV_VOZ_REY: play(QStringLiteral("N1_voz_por_el_rey")); break;
        case EV_VOZ_CUBRANSE: play(QStringLiteral("N1_voz_cubrance")); break;
        case EV_VOZ_POLVORA: play(QStringLiteral("N1_voz_polvora")); break;
        case EV_VOZ_RINDE: play(QStringLiteral("N3_voz_rinde")); break;
        case EV_VOZ_NO_ESCAPARAS: play(QStringLiteral("N3_voz_no_escaparas")); break;
        case EV_VOZ_FUEGO: play(QStringLiteral("N3_voz_fuego")); break;
        case EV_VOZ_CARGA: play(QStringLiteral("N3_voz_a_la_carga")); break;
        case EV_VOZ_ALIMANA: play(kAlimana); break;
        case EV_VOZ_IMPOSIBLE: play(kImposible, true); break;
        case EV_VOZ_SILENCIO: play(QStringLiteral("rocket_silencio")); break;
        case EV_VOZ_ESTALLIDO: play(QStringLiteral("rocket_estallido")); break;
        case EV_VICTORIA:
            tocar("recogida");
            if (nivel == NIVEL_PENON) {
                if (salidosTermino)
                    play(kMision, true);
                else
                    misionPendiente = true;
            }
            break;
        case EV_DERROTA: tocar("caida"); break;
        default: break;
        }
    }
}
