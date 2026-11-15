#ifndef SONIDO_H
#define SONIDO_H

#include <QHash>
#include <QObject>
#include <QSoundEffect>
#include <QString>
#include <vector>

class Sonido : public QObject {
    Q_OBJECT
private:
    QString carpeta;
    QHash<QString, QSoundEffect *> efectos;
    QSoundEffect *musica;
    bool activo;

    QSoundEffect *cargar(const QString &nombre);
    void tocar(const QString &nombre);

public:
    Sonido(const QString &carpetaBase, QObject *padre = nullptr);

    void iniciarMusica();
    void detenerMusica();
    void procesarEventos(const std::vector<short> &eventos);
    void setActivo(bool a);
};

#endif
