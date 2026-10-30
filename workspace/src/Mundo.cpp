/*
 * Practica 3 - Sistemas Informaticos Industriales
 * Juego Pong con FIFO y memoria compartida
 * Control automatico del jugador 1 mediante bot
 */

#include "Mundo.h"
#include "glut.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>

namespace
{
    int fd_fifo = -1;

    const int PUNTOS_PARA_GANAR = 3;

    // Nombre del archivo compartido con el bot
    const char* ARCHIVO_MEMORIA = "memoriaCompartida";

    // Enviar mensajes al logger mediante FIFO
    void enviarTexto(const char* texto)
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

    // Registrar los puntos de cada jugador
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

    // Finalizar cuando un jugador alcance 3 puntos
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

        // Cerrar FIFO para que logger termine
        if (fd_fifo >= 0)
        {
            close(fd_fifo);
            fd_fifo = -1;
        }

        // Finalizar tenis
        // Al salir se ejecuta el destructor global de CMundo
        std::exit(EXIT_SUCCESS);
    }
}

// CONSTRUCTOR Y DESTRUCTOR
CMundo::CMundo() : puntos1(0), puntos2(0), datos(NULL)
{
    Init();
}

CMundo::~CMundo()
{
    // Cerrar FIFO
    if (fd_fifo >= 0)
    {
        close(fd_fifo);
        fd_fifo = -1;
    }

    // Liberar la memoria compartida
    if (datos != NULL)
    {
        datos->~DatosMemCompartida();

        munmap(datos, sizeof(DatosMemCompartida));
        datos = NULL;
    }

    // Eliminar el archivo compartido
    unlink(ARCHIVO_MEMORIA);
}


// CONFIGURACION GRAFICA
void CMundo::InitGL()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glMatrixMode(GL_PROJECTION);
    gluPerspective(40.0, 800 / 600.0f, 0.1, 150);
}


// MOSTRAR TEXTO EN LA VENTANA
void print(char* texto, int posX, int posY,
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


// DIBUJAR EL JUEGO
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

    // Mostrar el marcador
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

    // Dibujar limites del campo
    fondo_izq.Dibuja();
    fondo_dcho.Dibuja();

    // Dibujar jugadores y pelota
    jug1.Dibuja();
    jug2.Dibuja();
    bola.Dibuja();

    glutSwapBuffers();
}


// MOVIMIENTO, REBOTES Y MEMORIA COMPARTIDA
void CMundo::OnTimer(int value)
{
    auto &jug1 = jugador1;
    auto &jug2 = jugador2;
    auto &bola = esfera;

    float tiempo = 0.025f;

    
    // MOVIMIENTO BOT
    if (datos != NULL)
    {
        if (datos->accion == 1)
        {
            // bot subir
            OnKeyboardDown('w', 0, 0);
        }
        else if (datos->accion == -1)
        {
            // bot bajar
            OnKeyboardDown('s', 0, 0);
        }
        else
        {
            // bot quieto
            jug1.velocidad.y = 0;
        }
    }

    
    // ACTUALIZAR MOVIMIENTOS
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

    // Rebotes con las raquetas
    jug1.Rebota(bola);
    jug2.Rebota(bola);

    // PUNTO PARA JUGADOR 2
    if (fondo_izq.Rebota(bola))
    {
        puntos2++;

        bola.centro.x = 0;
        bola.centro.y =
            std::rand() / (float)RAND_MAX;

        bola.velocidad.x =
            2 + 2 * std::rand() / (float)RAND_MAX;

        bola.velocidad.y =
            2 + 2 * std::rand() / (float)RAND_MAX;

        registrarPunto(2, puntos2);

        if (puntos2 >= PUNTOS_PARA_GANAR)
        {
            finalizarPartida(2);
        }
    }
    
    // PUNTO PARA JUGADOR 1
    if (fondo_dcho.Rebota(bola))
    {
        puntos1++;

        bola.centro.x = 0;
        bola.centro.y =
            std::rand() / (float)RAND_MAX;

        bola.velocidad.x =
            -2 - 2 * std::rand() / (float)RAND_MAX;

        bola.velocidad.y =
            -2 - 2 * std::rand() / (float)RAND_MAX;

        registrarPunto(1, puntos1);

        if (puntos1 >= PUNTOS_PARA_GANAR)
        {
            finalizarPartida(1);
        }
    }

  
    // ACTUALIZAR MEMORIA COMPARTIDA
    if (datos != NULL)
    {
        // Compartir la pelota
        datos->esfera = bola;

        // Compartir la raqueta del jugador 1
        datos->raqueta1 = jug1;

        // No modificamos datos->accion aqui:
        // ese valor lo escribe el bot
    }
}


// CONTROLES DEL TECLADO
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


// INICIALIZACION DEL MUNDO
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

  
    // CREAR EL ARCHIVO DE MEMORIA COMPARTIDA
    int fd_memoria = open(
        ARCHIVO_MEMORIA,
        O_CREAT | O_RDWR | O_TRUNC,
        0666
    );

    if (fd_memoria < 0)
    {
        std::perror("No se pudo crear la memoria compartida");
    }
    else
    {
        // Ajustar el tamaño del archivo
        if (ftruncate(fd_memoria, sizeof(DatosMemCompartida)) < 0)
        {
            std::perror("Error en ftruncate");
        }
        else
        {
            // Proyectar el archivo en memoria
            void* direccion = mmap(
                NULL,
                sizeof(DatosMemCompartida),
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                fd_memoria,
                0
            );

            if (direccion == MAP_FAILED)
            {
                std::perror("Error en mmap");
            }
            else
            {
                // Inicializar el objeto en memoria
                datos = new (direccion) DatosMemCompartida;

                datos->esfera = esfera;
                datos->raqueta1 = jugador1;
                datos->accion = 0;
            }
        }

        close(fd_memoria);
    }

    // ABRIR FIFO DE LA PRACTICA 2
    fd_fifo = open("FIFO", O_WRONLY);

    if (fd_fifo < 0)
    {
        std::perror("No se pudo abrir la FIFO");
    }
}
