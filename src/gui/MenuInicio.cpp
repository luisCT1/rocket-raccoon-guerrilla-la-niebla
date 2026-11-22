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
    botonEmboscada = new QPushButton(QStringLiteral("Nivel 1. Emboscada"));
    botonAsalto = new QPushButton(QStringLiteral("Nivel 2. Asalto"));
    botonPenon = new QPushButton(QStringLiteral("Nivel 3. El Peñón"));
    botonDificultad = new QPushButton();
    botonSalir = new QPushButton(QStringLiteral("Salir"));
    const QPushButton *botones[] = {botonCampana, botonEmboscada, botonAsalto, botonPenon, botonDificultad, botonSalir};
    for (const QPushButton *boton : botones) {
        const_cast<QPushButton *>(boton)->setStyleSheet(QString::fromLatin1(kBoton));
        const_cast<QPushButton *>(boton)->setCursor(Qt::PointingHandCursor);
        const_cast<QPushButton *>(boton)->setFocusPolicy(Qt::NoFocus);
    }

    auto *columna = new QVBoxLayout();
    columna->setSpacing(8);
    columna->addWidget(botonCampana);
    columna->addWidget(botonEmboscada);
    columna->addWidget(botonAsalto);
    columna->addWidget(botonPenon);
    columna->addWidget(botonDificultad);
    columna->addWidget(botonSalir);
    auto *atajo = new QLabel(QStringLiteral("Enter: campaña completa"));
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
    connect(botonEmboscada, &QPushButton::clicked, this, [this] { emit nivelElegido(NIVEL_EMBOSCADA); });
    connect(botonAsalto, &QPushButton::clicked, this, [this] { emit nivelElegido(NIVEL_ASALTO); });
    connect(botonPenon, &QPushButton::clicked, this, [this] { emit nivelElegido(NIVEL_PENON); });
    connect(botonDificultad, &QPushButton::clicked, this, [this] {
        dificultadActual = dificultadActual == DIF_NORMAL ? DIF_DIFICIL : DIF_NORMAL;
        actualizarTextoDificultad();
        emit dificultadCambiada();
    });
    connect(botonSalir, &QPushButton::clicked, this, &MenuInicio::salir);
    actualizarTextoDificultad();
}

short MenuInicio::dificultad() const {
    return dificultadActual;
}

void MenuInicio::actualizarTextoDificultad() {
    botonDificultad->setText(dificultadActual == DIF_DIFICIL ? QStringLiteral("Dificultad (nivel 3): Difícil")
                                                             : QStringLiteral("Dificultad (nivel 3): Normal"));
}

void MenuInicio::paintEvent(QPaintEvent *evento) {
    Q_UNUSED(evento);
    QPainter pintor(this);
    const QPixmap fondo(QStringLiteral(":/menu/fondo_menu.jpg"));
    if (!fondo.isNull())
        pintor.drawPixmap(rect(), fondo);
}
