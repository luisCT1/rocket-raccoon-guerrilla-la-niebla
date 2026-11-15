#include "MenuWidget.h"

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

MenuWidget::MenuWidget(QWidget *padre) : QWidget(padre) {
    setStyleSheet("MenuWidget { background: #1b2a2f; } QLabel { color: #e8e2c8; }"
                  "QPushButton { background: #3c5a4a; color: #f4f0dc; padding: 10px; font-size: 16px; border-radius: 4px; }"
                  "QPushButton:hover { background: #4f7560; }"
                  "QComboBox { font-size: 16px; padding: 4px; }");

    QLabel *titulo = new QLabel("Rocket Raccoon\nen la Guerrilla de la Niebla");
    titulo->setAlignment(Qt::AlignCenter);
    titulo->setStyleSheet("font-size: 34px; font-weight: bold;");

    QLabel *etDif = new QLabel("Dificultad");
    etDif->setAlignment(Qt::AlignCenter);
    dificultad = new QComboBox;
    dificultad->addItems({"Fácil", "Normal", "Difícil"});
    dificultad->setCurrentIndex(1);

    QPushButton *campania = new QPushButton("Campaña (niveles 1 a 3)");
    QPushButton *n1 = new QPushButton("Nivel 1: La emboscada");
    QPushButton *n2 = new QPushButton("Nivel 2: Asalto al campamento");
    QPushButton *n3 = new QPushButton("Nivel 3: El peñón");
    QPushButton *salirB = new QPushButton("Salir");

    QLabel *ayuda = new QLabel("A D W S / flechas: mover    Espacio: saltar    Shift: dash\n"
                               "Clic izquierdo: disparar    Clic derecho o G: granada    F: furia\n"
                               "P: pausa    Esc: menú");
    ayuda->setAlignment(Qt::AlignCenter);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(120, 40, 120, 40);
    lay->setSpacing(10);
    lay->addStretch();
    lay->addWidget(titulo);
    lay->addSpacing(20);
    lay->addWidget(etDif);
    lay->addWidget(dificultad);
    lay->addSpacing(10);
    lay->addWidget(campania);
    lay->addWidget(n1);
    lay->addWidget(n2);
    lay->addWidget(n3);
    lay->addWidget(salirB);
    lay->addSpacing(10);
    lay->addWidget(ayuda);
    lay->addStretch();

    connect(campania, &QPushButton::clicked, this, [this] { emit jugar(1, static_cast<short>(dificultad->currentIndex()), true); });
    connect(n1, &QPushButton::clicked, this, [this] { emit jugar(1, static_cast<short>(dificultad->currentIndex()), false); });
    connect(n2, &QPushButton::clicked, this, [this] { emit jugar(2, static_cast<short>(dificultad->currentIndex()), false); });
    connect(n3, &QPushButton::clicked, this, [this] { emit jugar(3, static_cast<short>(dificultad->currentIndex()), false); });
    connect(salirB, &QPushButton::clicked, this, &MenuWidget::salir);
}
