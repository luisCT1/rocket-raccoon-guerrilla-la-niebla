#ifndef ATLAS_NIVEL2_H
#define ATLAS_NIVEL2_H

#include <QPoint>
#include <QRect>

namespace Atlas {
constexpr int ROCKET_ANCHO_CELDA = 192;
constexpr int ROCKET_ALTO_CELDA = 136;
constexpr int ROCKET_PIE_X = 89;
constexpr int ROCKET_PIE_Y = 129;
constexpr int ROCKET_COLUMNAS = 6;
constexpr int ROCKET_FILAS = 11;
constexpr float ESCALA_ROCKET = 0.70f;
inline QRect celdaRocket(int fila, int col) {
    return QRect(col * ROCKET_ANCHO_CELDA, fila * ROCKET_ALTO_CELDA, ROCKET_ANCHO_CELDA, ROCKET_ALTO_CELDA);
}
constexpr int SOLDADO_ANCHO_CELDA = 192;
constexpr int SOLDADO_ALTO_CELDA = 160;
constexpr int SOLDADO_PIE_X = 63;
constexpr int SOLDADO_PIE_Y = 154;
constexpr int SOLDADO_COLUMNAS = 6;
constexpr int SOLDADO_FILAS = 10;
constexpr float ESCALA_SOLDADO = 0.60f;
inline QRect celdaSoldado(int fila, int col) {
    return QRect(col * SOLDADO_ANCHO_CELDA, fila * SOLDADO_ALTO_CELDA, SOLDADO_ANCHO_CELDA, SOLDADO_ALTO_CELDA);
}

enum FilaRocket {
    ROCKET_CAMINA_0 = 0,
    ROCKET_CAMINA_1 = 1,
    ROCKET_CAMINA_2 = 2,
    ROCKET_CAMINA_DER = 3,
    ROCKET_DISPARA = 4,
    ROCKET_DISPARA_Y_GRANADA = 5,
    ROCKET_GRANADA = 6,
    ROCKET_DASH = 7,
    ROCKET_DANIO = 8,
    ROCKET_LEVANTA = 9,
    ROCKET_REPOSO = 10
};
enum FilaSoldado {
    SOLDADO_CAMINA_0 = 0, SOLDADO_CAMINA_1 = 1, SOLDADO_CAMINA_2 = 2, SOLDADO_CAMINA_3 = 3,
    SOLDADO_DISPARA_0 = 4,
    SOLDADO_DISPARA_1 = 5,
    SOLDADO_ARMA = 6,
    SOLDADO_CAE = 7,
    SOLDADO_CAIDO = 8,
    SOLDADO_POSES = 9
};

inline const QRect TORRE_A(47, 20, 129, 239);
inline const QRect TORRE_CON_CANON(239, 14, 125, 245);
inline const QRect TORRE_B(425, 20, 126, 239);
inline const QRect TORRE_BANDERA(619, 20, 126, 233);
inline const QRect VALLA_LARGA(794, 71, 264, 104);
inline const QRect PUERTA(1096, 63, 236, 107);
inline const QRect VALLA_CORTA(1358, 63, 153, 101);
inline const QRect TIENDA_1(22, 273, 195, 135);
inline const QRect TIENDA_2(231, 300, 162, 110);
inline const QRect TIENDA_3(413, 289, 152, 115);
inline const QRect TIENDA_4(588, 272, 155, 121);
inline const QRect TIENDA_GRANDE(766, 227, 243, 180);
inline const QRect CANON_1(1034, 269, 142, 121);
inline const QRect CANON_2(1206, 263, 134, 117);
inline const QRect CANON_3(1370, 262, 140, 117);
inline const QRect BALAS_CANON_MONTON(25, 454, 70, 58);
inline const QRect CAJON_BALAS(126, 444, 115, 94);
inline const QRect BALAS_CANON_POCAS(254, 454, 56, 45);
inline const QRect BARRIL_1(332, 443, 61, 91);
inline const QRect BARRIL_2(425, 449, 58, 84);
inline const QRect BARRIL_ROJO(516, 449, 58, 85);
inline const QRect CAJA_1(606, 439, 74, 86);
inline const QRect CAJA_2(698, 442, 75, 82);
inline const QRect SACOS_1(790, 443, 139, 73);
inline const QRect SACOS_2(947, 449, 174, 69);
inline const QRect CARRETA(1152, 406, 186, 149);
inline const QRect CAJAS_Y_CANON(1366, 418, 141, 97);
inline const QRect CABALLO_FRISA_1(30, 578, 98, 76);
inline const QRect CABALLO_FRISA_2(149, 579, 96, 75);
inline const QRect EMPALIZADA_1(276, 566, 139, 87);
inline const QRect EMPALIZADA_2(449, 569, 186, 85);
inline const QRect CERCA_TRAVESANOS(662, 569, 197, 82);
inline const QRect FAROL_POSTE(914, 536, 70, 140);
inline const QRect BANDERA_REAL_ROJA(1060, 550, 67, 139);
inline const QRect BANDERA_CRUZ_ROJA(1175, 555, 73, 136);
inline const QRect BANDERA_TRICOLOR(1298, 555, 84, 138);
inline const QRect POSTE_INDICADOR(1438, 567, 75, 126);
inline const QRect ARBOL_1(26, 673, 184, 157);
inline const QRect ARBOL_2(230, 669, 197, 161);
inline const QRect ARBOL_3(510, 695, 143, 130);
inline const QRect ARBUSTO_1(417, 753, 92, 69);
inline const QRect ARBUSTO_2(696, 715, 78, 87);
inline const QRect ROCAS(790, 714, 136, 99);
inline const QRect RUINAS_1(942, 698, 166, 121);
inline const QRect TRONCOS_1(1135, 730, 151, 84);
inline const QRect TRONCOS_2(1309, 724, 123, 90);
inline const QRect TOCON(1453, 748, 67, 60);
inline const QRect RUINAS_2(18, 854, 144, 140);
inline const QRect MUELLE(198, 876, 172, 125);
inline const QRect FAROL_POSTE_2(391, 846, 77, 149);
inline const QRect CAJA_FAROL(519, 872, 89, 102);
inline const QRect BALAS_CANON_MONTON_2(640, 930, 95, 60);
inline const QRect HUMO_1(745, 861, 119, 127);
inline const QRect HUMO_2(899, 881, 94, 105);
inline const QRect EXPLOSION_1(1027, 856, 153, 145);
inline const QRect EXPLOSION_2_FUEGO(1215, 840, 118, 161);
inline const QRect EXPLOSION_3(1365, 845, 156, 156);

}

#endif
