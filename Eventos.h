#ifndef EVENTOS_H
#define EVENTOS_H

enum EventoJuego : short {
    EV_DISPARO_ROCKET = 0,
    EV_GRANADA,
    EV_EXPLOSION,
    EV_DASH,
    EV_GOLPE_A_ROCKET,
    EV_FURIA_ACTIVADA,
    EV_TAREA_COMPLETADA,
    EV_ALARMA,
    EV_CANON_DISPARA,
    EV_CAMBIO_FASE_JEFE,
    EV_ATAQUE_GIRATORIO,
    EV_VICTORIA,
    EV_DERROTA
};

enum NivelDificultad : short { DIF_FACIL = 0, DIF_NORMAL = 1, DIF_DIFICIL = 2 };

enum NumeroNivel : short { NIVEL_EMBOSCADA = 1, NIVEL_ASALTO = 2, NIVEL_PENON = 3 };

#endif
