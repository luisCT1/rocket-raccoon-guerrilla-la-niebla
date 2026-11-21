#ifndef VENTANA_JUEGO_H
#define VENTANA_JUEGO_H

#include <QMainWindow>

class QGraphicsView;
class QLabel;
class QPushButton;
class QStackedWidget;
class QTimer;
class EscenaMision;
class MenuInicio;

class VentanaJuego : public QMainWindow {
    Q_OBJECT

public:
    explicit VentanaJuego(QWidget *parent = nullptr);
    ~VentanaJuego() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void tick();
    void empezarCampana();
    void empezarUno(int numero);
    void volverAlMenu();
    void siguienteNivel();

private:
    void encuadrar();
    void soltarTeclas();
    void abrirNivel(int numero);
    void colocarAvisos();
    void mostrarResultado();

    QStackedWidget *paginas = nullptr;
    MenuInicio *menu = nullptr;
    QGraphicsView *vista = nullptr;
    QLabel *ayuda = nullptr;
    QLabel *mision = nullptr;
    QLabel *aviso = nullptr;
    QPushButton *volver = nullptr;
    QPushButton *siguiente = nullptr;
    QTimer *reloj = nullptr;
    EscenaMision *escena = nullptr;
    bool enNivel = false;
    bool campana = false;
    bool resultadoVisible = false;
    int nivelCurso = 0;
    float tiempoMision = 0.f;
};

#endif
