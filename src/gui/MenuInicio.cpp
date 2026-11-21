#include "MenuInicio.h"

#include "Eventos.h"

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
    "  padding: 8px 18px;"
    "  font-size: 16px;"
    "  font-family: Georgia;"
    "  min-width: 220px;"
    "}"
    "QPushButton:hover { background: rgba(46, 72, 48, 235); border-color: #f0d090; }"
    "QPushButton:pressed { background: rgba(18, 28, 22, 240); }";
}

MenuInicio::MenuInicio(QWidget *parent) : QWidget(parent) {
    botonCampana = new QPushButton(QStringLiteral("Campaña"));
    botonJugar = new QPushButton(QStringLiteral("Jugar un nivel"));
    botonEmboscada = new QPushButton(QStringLiteral("Nivel 1. Emboscada"));
    botonAsalto = new QPushButton(QStringLiteral("Nivel 2. Asalto"));
    botonPenon = new QPushButton(QStringLiteral("Nivel 3. El Peñón"));
    botonVolver = new QPushButton(QStringLiteral("Volver"));
    botonSalir = new QPushButton(QStringLiteral("Salir"));
    const QPushButton *botones[] = {botonCampana, botonJugar, botonEmboscada, botonAsalto, botonPenon, botonVolver, botonSalir};
    for (const QPushButton *boton : botones) {
        const_cast<QPushButton *>(boton)->setStyleSheet(QString::fromLatin1(kBoton));
        const_cast<QPushButton *>(boton)->setCursor(Qt::PointingHandCursor);
        const_cast<QPushButton *>(boton)->setFocusPolicy(Qt::NoFocus);
        connect(const_cast<QPushButton *>(boton), &QPushButton::clicked, this, &MenuInicio::pulso);
    }

    auto *columna = new QVBoxLayout();
    columna->setSpacing(8);
    columna->addWidget(botonCampana);
    columna->addWidget(botonJugar);
    columna->addWidget(botonEmboscada);
    columna->addWidget(botonAsalto);
    columna->addWidget(botonPenon);
    columna->addWidget(botonVolver);
    columna->addWidget(botonSalir);
    atajo = new QLabel(QStringLiteral("Enter: campaña completa"));
    atajo->setAlignment(Qt::AlignRight);
    atajo->setStyleSheet(QStringLiteral(
        "color: #f4efe2; font-size: 13px; font-family: Georgia; background: rgba(22, 32, 24, 170); padding: 2px 8px;"));
    columna->addWidget(atajo);

    auto *fila = new QHBoxLayout();
    fila->addStretch(1);
    fila->addLayout(columna);

    auto *caja = new QVBoxLayout(this);
    caja->setContentsMargins(48, 24, 180, 36);
    caja->addStretch(5);
    caja->addLayout(fila);

    connect(botonCampana, &QPushButton::clicked, this, &MenuInicio::campana);
    connect(botonJugar, &QPushButton::clicked, this, &MenuInicio::mostrarNiveles);
    connect(botonEmboscada, &QPushButton::clicked, this, [this] { emit nivelElegido(NIVEL_EMBOSCADA); });
    connect(botonAsalto, &QPushButton::clicked, this, [this] { emit nivelElegido(NIVEL_ASALTO); });
    connect(botonPenon, &QPushButton::clicked, this, [this] { emit nivelElegido(NIVEL_PENON); });
    connect(botonVolver, &QPushButton::clicked, this, &MenuInicio::mostrarRaiz);
    connect(botonSalir, &QPushButton::clicked, this, &MenuInicio::salir);
    mostrarRaiz();
}

bool MenuInicio::estaEligiendoNivel() const {
    return eligiendoNivel;
}

void MenuInicio::mostrarRaiz() {
    eligiendoNivel = false;
    botonCampana->show();
    botonJugar->show();
    botonSalir->show();
    atajo->show();
    botonEmboscada->hide();
    botonAsalto->hide();
    botonPenon->hide();
    botonVolver->hide();
}

void MenuInicio::mostrarNiveles() {
    eligiendoNivel = true;
    botonCampana->hide();
    botonJugar->hide();
    botonSalir->hide();
    atajo->hide();
    botonEmboscada->show();
    botonAsalto->show();
    botonPenon->show();
    botonVolver->show();
}

void MenuInicio::paintEvent(QPaintEvent *evento) {
    Q_UNUSED(evento);
    QPainter pintor(this);
    const QPixmap fondo(QStringLiteral(":/menu/fondo_menu.jpg"));
    if (!fondo.isNull())
        pintor.drawPixmap(rect(), fondo);
}
