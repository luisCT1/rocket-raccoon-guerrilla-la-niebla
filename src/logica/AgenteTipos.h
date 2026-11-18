#ifndef AGENTETIPOS_H
#define AGENTETIPOS_H

enum AccionCapitan : short {
    ACC_NINGUNA = -1,
    ACC_ACOSAR = 0,
    ACC_CANON,
    ACC_GIRATORIO,
    ACC_TROMBA,
    ACC_REPLEGARSE,
    ACC_NUM
};

enum TipoEstimulo : short { ESTIM_NINGUNO = 0, ESTIM_VISTA, ESTIM_DASH, ESTIM_GRANADA };

enum HabitoJugador : short { HAB_DASH = 0, HAB_GRANADA, HAB_ALTO, HAB_LEJOS, HAB_NUM };

enum FaseJefe : short { FASE_ACOSA = 1, FASE_CANON = 2, FASE_AGRESIVO = 3 };

#endif
