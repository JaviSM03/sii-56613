#pragma once

#include "Esfera.h"
#include "Raqueta.h"

// Datos compartidos entre el juego y el bot
class DatosMemCompartida
{
public:
    Esfera esfera;
    Raqueta raqueta1;
    int accion; // 1 subir, 0 parar, -1 bajar
};
