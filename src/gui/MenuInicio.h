#ifndef MENU_INICIO_H
#define MENU_INICIO_H

#include <QWidget>

class QLabel;
class QPushButton;

class MenuInicio : public QWidget {
    Q_OBJECT

public:
    explicit MenuInicio(QWidget *parent = nullptr);
    bool estaEligiendoNivel() const;
    void mostrarRaiz();

protected:
    void paintEvent(QPaintEvent *evento) override;

signals:
    void campana();
    void nivelElegido(int numero);
    void salir();
    void pulso();

private:
    void mostrarNiveles();

    QPushButton *botonCampana = nullptr;
    QPushButton *botonJugar = nullptr;
    QPushButton *botonEmboscada = nullptr;
    QPushButton *botonAsalto = nullptr;
    QPushButton *botonPenon = nullptr;
    QPushButton *botonVolver = nullptr;
    QPushButton *botonSalir = nullptr;
    QLabel *atajo = nullptr;
    bool eligiendoNivel = false;
};

#endif
