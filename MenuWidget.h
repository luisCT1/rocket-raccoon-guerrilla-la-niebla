#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QWidget>

class QComboBox;

class MenuWidget : public QWidget {
    Q_OBJECT
private:
    QComboBox *dificultad;

public:
    explicit MenuWidget(QWidget *padre = nullptr);

signals:
    void jugar(short nivel, short dificultad, bool campania);
    void salir();
};

#endif
