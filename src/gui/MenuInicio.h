#ifndef MENU_INICIO_H
#define MENU_INICIO_H

#include <QWidget>

class QPushButton;

class MenuInicio : public QWidget {
    Q_OBJECT

public:
    explicit MenuInicio(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *evento) override;

signals:
    void jugar();
    void salir();

private:
    QPushButton *botonJugar = nullptr;
    QPushButton *botonSalir = nullptr;
};

#endif
