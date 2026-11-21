#include "Sonido.h"
#include "Eventos.h"

#include <QAudioOutput>
#include <QDir>
#include <QFile>
#include <QMediaPlayer>
#include <QSoundEffect>
#include <QString>
#include <QUrl>

Sonido::Sonido(QObject *padre_) : padre(padre_) {
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

    const char *aliasVoz[] = {
        "voz_fuego.wav", "voz_rey.wav", "voz_rinde.wav",
        "voz_penon.wav", "voz_sable.wav", "voz_canon.wav"};
    for (int i = 0; i < 6; ++i)
        rutasVoz[i] = extraer(aliasVoz[i], aliasVoz[i]);
    vozSalida = new QAudioOutput(padre);
    vozSalida->setVolume(1.f);
    voz = new QMediaPlayer(padre);
    voz->setAudioOutput(vozSalida);
    QMediaPlayer *reproductor = voz;
    QObject::connect(voz, &QMediaPlayer::mediaStatusChanged, padre,
                     [reproductor](QMediaPlayer::MediaStatus estado) {
                         if (estado == QMediaPlayer::LoadedMedia
                             && reproductor->playbackState() != QMediaPlayer::PlayingState)
                             reproductor->play();
                     });
}

void Sonido::poner(const char *clave, const char *archivo, float volumen) {
    auto *efecto = new QSoundEffect(padre);
    efecto->setSource(QUrl(QStringLiteral("qrc:/sonidos/") + QString::fromLatin1(archivo)));
    efecto->setVolume(volumen);
    clips.insert(QString::fromLatin1(clave), efecto);
}

QString Sonido::extraer(const char *alias, const char *nombre) {
    QFile origen(QStringLiteral(":/sonidos/") + QString::fromLatin1(alias));
    if (!origen.open(QIODevice::ReadOnly))
        return {};
    const QString ruta = QDir::temp().filePath(QStringLiteral("niebla-") + QString::fromLatin1(nombre));
    QFile destino(ruta);
    if (!destino.open(QIODevice::WriteOnly))
        return {};
    destino.write(origen.readAll());
    return ruta;
}

void Sonido::decir(int cual) {
    if (!voz || cual < 0 || cual > 5 || rutasVoz[cual].isEmpty())
        return;
    voz->stop();
    voz->setPlaybackRate(1.f);
    voz->setSource(QUrl::fromLocalFile(rutasVoz[cual]));
    voz->play();
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
        case EV_DISPARO_ENEMIGO: tocar("enemigo"); break;
        case EV_GRANADA: tocar("granada"); break;
        case EV_EXPLOSION: tocar("explosion"); break;
        case EV_DASH: tocar("impulso"); break;
        case EV_SALTO: tocar("salto"); break;
        case EV_GOLPE_A_ROCKET: tocar("golpe"); break;
        case EV_CAIDA: tocar("caida"); break;
        case EV_FURIA_ACTIVADA: tocar("furia"); break;
        case EV_ENEMIGO_CAE: tocar("cuerpo"); break;
        case EV_CORREO_HUYE: tocar("correo"); break;
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
        case EV_ATAQUE_GIRATORIO: tocar("sable"); break;
        case EV_SABLE: tocar("sable"); break;
        case EV_TROMBA: tocar("viento"); tocar("sable"); break;
        case EV_VIENTO: tocar("viento"); break;
        case EV_RELINCHO: tocar("relincho"); break;
        case EV_VOZ:
            decir(vozTurno % 6);
            vozTurno = (vozTurno + 1) % 6;
            break;
        case EV_CAMBIO_FASE_JEFE: tocar("alarma"); break;
        case EV_VICTORIA: tocar("recogida"); break;
        case EV_DERROTA: tocar("caida"); break;
        default: break;
        }
    }
}
