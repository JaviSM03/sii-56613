// Mundo.h - Practica 3 SII
// Comunicacion mediante memoria compartida

#if !defined(AFX_MUNDO_H__9510340A_3D75_485F_93DC_302A43B8039A__INCLUDED_)
#define AFX_MUNDO_H__9510340A_3D75_485F_93DC_302A43B8039A__INCLUDED_

#include <vector>
#include "Plano.h"
#include "DatosMemCompartida.h"

#if _MSC_VER > 1000
#pragma once
#endif

class CMundo
{
public:
    CMundo();
    virtual ~CMundo();

    void Init();
    void InitGL();
    void OnKeyboardDown(unsigned char key, int x, int y);
    void OnTimer(int value);
    void OnDraw();

    // Elementos del juego
    Esfera esfera;
    std::vector<Plano> paredes;

    Plano fondo_izq;
    Plano fondo_dcho;

    Raqueta jugador1;
    Raqueta jugador2;

    // Puntuaciones
    int puntos1;
    int puntos2;

    // Memoria compartida con el bot
    DatosMemCompartida* datos;
};

#endif
