#ifndef SONIDO_H
#define SONIDO_H

#include <QElapsedTimer>
#include <QHash>
#include <QSet>
#include <QString>
#include <vector>

class QAudioOutput;
class QMediaPlayer;
class QObject;
class QSoundEffect;

class Sonido {
public:
    explicit Sonido(QObject *padre, short nivel = 0);
    ~Sonido();

    bool play(const QString &nombre, bool prioridad = false);
    void eventos(const std::vector<short> &lista, bool furioso);

private:
    void poner(const char *clave, const char *archivo, float volumen);
    void tocar(const char *clave);
    QString extraer(const QString &archivo);
    bool esMp3(const QString &recurso) const;
    QString extraerMp3Disfrazado(const QString &nombre);
    bool esFrecuente(const QString &nombre) const;
    bool reproducirCapitan(const QString &nombre, bool prioridad);
    bool reproducirRocket(const QString &nombre);
    void cerrarSalidos();
    QSoundEffect *efectoDe(const QString &nombre);

    QObject *padre;
    short nivel;
    QHash<QString, QSoundEffect *> clips;
    QHash<QString, QSoundEffect *> vocesCapitan;
    QHash<QString, QString> rutasRocket;
    QSet<QString> avisados;
    QMediaPlayer *vozRocket = nullptr;
    QAudioOutput *salidaRocket = nullptr;
    QMediaPlayer *vozCapitan = nullptr;
    QAudioOutput *salidaCapitan = nullptr;
    QSoundEffect *capitanSonando = nullptr;
    QString rocketNombre;
    QElapsedTimer reloj;
    qint64 ultimoCapitan = -100000;
    qint64 ultimoRocket = -100000;
    qint64 ultimoFrecuente = -100000;
    qint64 ultimoAlimana = -100000;
    bool respetoPendiente = false;
    bool alimanaSono = false;
    bool salidosPedido = false;
    bool salidosTermino = false;
    bool misionPendiente = false;
    bool cambiandoRocket = false;
};

#endif
