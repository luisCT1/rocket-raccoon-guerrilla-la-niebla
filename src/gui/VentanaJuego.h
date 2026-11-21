#ifndef VENTANA_JUEGO_H
#define VENTANA_JUEGO_H

#include <QMainWindow>
#include <QString>

class QAudioOutput;
class QGraphicsView;
class QLabel;
class QMediaPlayer;
class QPushButton;
class QSoundEffect;
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
    void reintentarNivel();

private:
    void encuadrar();
    void soltarTeclas();
    void sincronizarTeclas();
    void abrirNivel(int numero);
    void colocarAvisos();
    void mostrarResultado();
    void prepararMusica();
    void pedirMusica(int pista);
    void avanzarMusica(float dt, bool furiosa);
    void cargarPista(int reproductor, int pista);
    void tocarClic();
    float topeMusica(int pista, bool furiosa) const;

    QStackedWidget *paginas = nullptr;
    MenuInicio *menu = nullptr;
    QGraphicsView *vista = nullptr;
    QLabel *ayuda = nullptr;
    QLabel *mision = nullptr;
    QLabel *aviso = nullptr;
    QPushButton *volver = nullptr;
    QPushButton *siguiente = nullptr;
    QPushButton *reintentar = nullptr;
    QTimer *reloj = nullptr;
    EscenaMision *escena = nullptr;
    bool enNivel = false;
    bool campana = false;
    bool resultadoVisible = false;
    int nivelCurso = 0;
    float tiempoMision = 0.f;
    QMediaPlayer *tema[2] = {nullptr, nullptr};
    QAudioOutput *voz[2] = {nullptr, nullptr};
    QSoundEffect *clic = nullptr;
    QString rutasMusica[4];
    int pistaDe[2] = {0, 0};
    int reproductorActivo = 0;
    float cruceMusica = 1.f;
    float volumenSalida = 0.f;
    bool fundiendo = false;
};

#endif
