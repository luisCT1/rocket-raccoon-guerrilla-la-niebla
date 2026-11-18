#include "MenuInicio.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QPushButton>
#include <QVBoxLayout>

namespace {
const char *kBoton =
    "QPushButton {"
    "  color: #f4efe2;"
    "  background: rgba(28, 42, 32, 225);"
    "  border: 2px solid #e2b15a;"
    "  border-radius: 8px;"
    "  padding: 11px 26px;"
    "  font-size: 18px;"
    "  min-width: 180px;"
    "}"
    "QPushButton:hover { background: rgba(46, 72, 48, 235); border-color: #f0d090; }"
    "QPushButton:pressed { background: rgba(18, 28, 22, 240); }";
}

MenuInicio::MenuInicio(QWidget *parent) : QWidget(parent) {
    botonJugar = new QPushButton(QStringLiteral("Jugar"));
    botonSalir = new QPushButton(QStringLiteral("Salir"));
    botonJugar->setStyleSheet(QString::fromLatin1(kBoton));
    botonSalir->setStyleSheet(QString::fromLatin1(kBoton));
    botonJugar->setCursor(Qt::PointingHandCursor);
    botonSalir->setCursor(Qt::PointingHandCursor);
    botonJugar->setFocusPolicy(Qt::NoFocus);
    botonSalir->setFocusPolicy(Qt::NoFocus);

    auto *columna = new QVBoxLayout();
    columna->setSpacing(10);
    columna->addWidget(botonJugar);
    columna->addWidget(botonSalir);
    auto *atajo = new QLabel(QStringLiteral("Enter también empieza"));
    atajo->setAlignment(Qt::AlignRight);
    atajo->setStyleSheet(QStringLiteral(
        "color: #f4efe2; font-size: 13px; background: rgba(22, 32, 24, 170); padding: 2px 8px;"));
    columna->addWidget(atajo);

    auto *fila = new QHBoxLayout();
    fila->addStretch(1);
    fila->addLayout(columna);

    auto *caja = new QVBoxLayout(this);
    caja->setContentsMargins(48, 24, 210, 68);
    caja->addStretch(5);
    caja->addLayout(fila);

    connect(botonJugar, &QPushButton::clicked, this, &MenuInicio::jugar);
    connect(botonSalir, &QPushButton::clicked, this, &MenuInicio::salir);
}

void MenuInicio::paintEvent(QPaintEvent *evento) {
    Q_UNUSED(evento);
    QPainter pintor(this);
    const QPixmap fondo(QStringLiteral(":/menu/fondo_menu.jpg"));
    if (!fondo.isNull())
        pintor.drawPixmap(rect(), fondo);
}
