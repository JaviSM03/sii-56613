// Raqueta.cpp: implementation of the Raqueta class.
//
//////////////////////////////////////////////////////////////////////

#include "Raqueta.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Raqueta::Raqueta()
{
	
}

Raqueta::~Raqueta()
{

}

void Raqueta::Mueve(float t)
{
x1 = x1 + velocidad.x * t;
x2 = x2 + velocidad.x * t;
y1 = y1 + velocidad.y * t;
y2 = y2 + velocidad.y * t;
}
