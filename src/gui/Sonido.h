#ifndef SONIDO_H
#define SONIDO_H

#include <QHash>
#include <QString>
#include <vector>

class QAudioOutput;
class QMediaPlayer;
class QObject;
class QSoundEffect;

class Sonido {
public:
    explicit Sonido(QObject *padre);
    void eventos(const std::vector<short> &lista, bool furioso);

private:
    void poner(const char *clave, const char *archivo, float volumen);
    void tocar(const char *clave);
    void decir(int cual);
    QString extraer(const char *alias, const char *nombre);

    QObject *padre;
    QHash<QString, QSoundEffect *> clips;
    QMediaPlayer *voz = nullptr;
    QAudioOutput *vozSalida = nullptr;
    QString rutasVoz[6];
    int vozTurno = 0;
};

#endif
