/*
 * Práctica 2 - Sistemas Informáticos Industriales - Javier Serrano Molina 
 * Juego Pong con comunicación FIFO
 * Finalización de la partida automática cuando uno de los 2 jugadores llega a 3 puntos
 */

#include <fstream>
#include "Mundo.h"
#include "glut.h"

#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace
{
    int fd_fifo = -1;

    // Puntos necesarios para ganar
    const int PUNTOS_PARA_GANAR = 3;

    // Mostrar el texto y enviarlo a logger
    void enviarTexto(const char *texto)
    {
        
        if (fd_fifo >= 0)
        {
            ssize_t resultado = write(
                fd_fifo,
                texto,
                std::strlen(texto) + 1
            );

            if (resultado < 0)
                std::perror("Error al escribir en FIFO");
        }
    }

    // Notificar los puntos de cada jugador
    void registrarPunto(int jugador, int total)
    {
        char aviso[128];

        std::snprintf(
            aviso,
            sizeof(aviso),
            "Jugador %d marca 1 punto, lleva un total de %d puntos\n",
            jugador,
            total
        );

        enviarTexto(aviso);
    }

    // Terminar el juego cuando exista un ganador
    void finalizarPartida(int ganador)
    {
        char avisoFinal[128];

        std::snprintf(
            avisoFinal,
            sizeof(avisoFinal),
            "\nFIN DE LA PARTIDA - GANADOR: JUGADOR %d\n",
            ganador
        );

        enviarTexto(avisoFinal);

        // Cerrar FIFO para que logger reciba EOF
        if (fd_fifo >= 0)
        {
            close(fd_fifo);
            fd_fifo = -1;
        }

        // Finalizar el proceso tenis
        std::exit(EXIT_SUCCESS);
    }
}

CMundo::CMundo() : puntos1(0), puntos2(0)
{
    Init();
}

CMundo::~CMundo()
{
    if (fd_fifo >= 0)
    {
        close(fd_fifo);
        fd_fifo = -1;
    }
}

void CMundo::InitGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(40.0, 800 / 600.0f, 0.1, 150);
}

void print(char *texto, int posX, int posY,
           float rojo, float verde, float azul)
{
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_TEXTURE);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    gluOrtho2D(
        0, glutGet(GLUT_WINDOW_WIDTH),
        0, glutGet(GLUT_WINDOW_HEIGHT)
    );

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    glColor3f(rojo, verde, azul);

    glRasterPos3f(
        posX,
        glutGet(GLUT_WINDOW_HEIGHT) - 18 - posY,
        0
    );

    int longitud = std::strlen(texto);

    for (int letra = 0; letra < longitud; ++letra)
    {
        glutBitmapCharacter(
            GLUT_BITMAP_HELVETICA_18,
            texto[letra]
        );
    }

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
}

void CMundo::OnDraw()
{
    auto &jug1 = jugador1;
    auto &jug2 = jugador2;
    auto &bola = esfera;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(
        0.0, 0.0, 17.0,
        0.0, 0.0, 0.0,
        0.0, 1.0, 0.0
    );

    // Mostrar marcador
    char marcador[100];

    std::snprintf(
        marcador, sizeof(marcador),
        "Jugador1: %d", puntos1
    );

    print(marcador, 10, 0, 1, 1, 1);

    std::snprintf(
        marcador, sizeof(marcador),
        "Jugador2: %d", puntos2
    );

    print(marcador, 650, 0, 1, 1, 1);

    // Dibujar paredes
    for (auto &pared : paredes)
    {
        pared.Dibuja();
    }

    // Dibujar limites
    fondo_izq.Dibuja();
    fondo_dcho.Dibuja();

    // Dibujar jugadores y pelota
    jug1.Dibuja();
    jug2.Dibuja();
    bola.Dibuja();

    glutSwapBuffers();
}

void CMundo::OnTimer(int value)
{
    auto &jug1 = jugador1;
    auto &jug2 = jugador2;
    auto &bola = esfera;

    const float tiempo = 0.025f;

    // Actualizar movimientos
    jug1.Mueve(tiempo);
    jug2.Mueve(tiempo);
    bola.Mueve(tiempo);

    // Rebotes contra las paredes
    for (auto &pared : paredes)
    {
        pared.Rebota(bola);
        pared.Rebota(jug1);
        pared.Rebota(jug2);
    }

    // Rebotes contra las raquetas
    jug1.Rebota(bola);
    jug2.Rebota(bola);

    if (fondo_izq.Rebota(bola))
    {
        puntos2++;

        // Reiniciar posicion
        bola.centro.x = 0;
        bola.centro.y =
            std::rand() / (float)RAND_MAX;

        // Nueva velocidad hacia la derecha
        bola.velocidad.x =
            2 + 2 * std::rand() / (float)RAND_MAX;

        bola.velocidad.y =
            2 + 2 * std::rand() / (float)RAND_MAX;

        // Avisar al logger
        registrarPunto(2, puntos2);

        // Comprobar si ha ganado
        if (puntos2 >= PUNTOS_PARA_GANAR)
        {
            finalizarPartida(2);
        }
    }

    if (fondo_dcho.Rebota(bola))
    {
        puntos1++;

        // Reiniciar posicion
        bola.centro.x = 0;
        bola.centro.y =
            std::rand() / (float)RAND_MAX;

        // Nueva velocidad hacia la izquierda
        bola.velocidad.x =
            -2 - 2 * std::rand() / (float)RAND_MAX;

        bola.velocidad.y =
            -2 - 2 * std::rand() / (float)RAND_MAX;

        // Avisar al logger
        registrarPunto(1, puntos1);

        // Comprobar si ha ganado
        if (puntos1 >= PUNTOS_PARA_GANAR)
        {
            finalizarPartida(1);
        }
    }
}

void CMundo::OnKeyboardDown(unsigned char tecla,
                             int x, int y)
{
    auto &vel1 = jugador1.velocidad;
    auto &vel2 = jugador2.velocidad;

    switch (tecla)
    {
        // Jugador 1
        case 'w':
            vel1.y = 4;
            break;

        case 's':
            vel1.y = -4;
            break;

        // Jugador 2
        case 'o':
            vel2.y = 4;
            break;

        case 'l':
            vel2.y = -4;
            break;
    }
}

void CMundo::Init()
{
    auto &jug1 = jugador1;
    auto &jug2 = jugador2;

    Plano borde;

    // Pared inferior
    borde.x1 = -7;
    borde.y1 = -5;
    borde.x2 = 7;
    borde.y2 = -5;
    paredes.push_back(borde);

    // Pared superior
    borde.x1 = -7;
    borde.y1 = 5;
    borde.x2 = 7;
    borde.y2 = 5;
    paredes.push_back(borde);

    // Limite izquierdo
    fondo_izq.r = 0;
    fondo_izq.x1 = -7;
    fondo_izq.y1 = -5;
    fondo_izq.x2 = -7;
    fondo_izq.y2 = 5;

    // Limite derecho
    fondo_dcho.r = 0;
    fondo_dcho.x1 = 7;
    fondo_dcho.y1 = -5;
    fondo_dcho.x2 = 7;
    fondo_dcho.y2 = 5;

    // Jugador izquierdo
    jug1.g = 0;
    jug1.x1 = -6;
    jug1.y1 = -1;
    jug1.x2 = -6;
    jug1.y2 = 1;

    // Jugador derecho
    jug2.g = 0;
    jug2.x1 = 6;
    jug2.y1 = -1;
    jug2.x2 = 6;
    jug2.y2 = 1;

    // Abrir FIFO en modo escritura
    fd_fifo = open("FIFO", O_WRONLY);

    if (fd_fifo < 0)
    {
        std::perror("No se pudo abrir la FIFO");
    }
}
