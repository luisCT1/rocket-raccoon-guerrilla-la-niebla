#ifndef SONIDO_H
#define SONIDO_H

#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QString>
#include <QVector>

#include <vector>

class QMediaPlayer;
class QSoundEffect;
class QTimer;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QAudioOutput;
#endif

class Sonido : public QObject {
    Q_OBJECT

public:
    explicit Sonido(QObject *padre = nullptr);

    void iniciarMenu();
    void iniciarNivel(int numero);
    void detenerMusica();
    void tocar(const QString &nombre, float volumen = 1.f);
    void tocarLuego(const QString &nombre, int retrasoMs, float volumen = 1.f);
    void ritmo(const QString &nombre, bool activo, int cadaMs);
    void procesar(const std::vector<short> &eventos);

private:
    void cargarEfectos();
    void ponerMusica(const QString &archivo);
    void sonarAmbiente();

    QString carpeta;
    bool listo = false;
    int nivelActual = 0;
    int golpesRecibidos = 0;
    QElapsedTimer cronometro;
    QHash<QString, QVector<QSoundEffect *>> efectos;
    QHash<QString, int> turno;
    QHash<QString, qint64> ultimo;
    QHash<QString, qint64> ritmos;
    QMediaPlayer *musica = nullptr;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QAudioOutput *salida = nullptr;
#endif
    QString musicaActual;
    QTimer *ambiente = nullptr;
};

#endif
