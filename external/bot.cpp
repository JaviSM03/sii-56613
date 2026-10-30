/*

 * Practica 3 - Sistemas Informaticos Industriales
 * Bot automatico para el juego Pong
 * Comunicacion mediante memoria compartida
 
 */

#include "DatosMemCompartida.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <cstdio>

int main()
{
    // Nombre del archivo utilizado por Mundo.cpp
    const char* ARCHIVO_MEMORIA = "memoriaCompartida";

    // Descriptor y puntero a los datos compartidos
    int fd_memoria;
    DatosMemCompartida* dat;

    // Abrir el archivo compartido
    fd_memoria = open(ARCHIVO_MEMORIA, O_RDWR);

    if (fd_memoria < 0)
    {
        perror("No se pudo abrir la memoria compartida");
        return 1;
    }

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
        perror("Error al proyectar la memoria");
        close(fd_memoria);
        return 1;
    }

    // Cerrar el descriptor y guardar el puntero
    close(fd_memoria);

    dat = (DatosMemCompartida*)direccion;

    printf("Bot conectado al juego correctamente\n");

    // Bucle principal del bot
    while (1)
    {
        usleep(25000);

        // Detectar si el juego ha terminado
        if (access(ARCHIVO_MEMORIA, F_OK) != 0)
        {
            break;
        }

        // Posicion vertical de la pelota
        float posBola = dat->esfera.centro.y;

        // Centro vertical de la raqueta
        float centroRaqueta =
            (dat->raqueta1.y1 + dat->raqueta1.y2) / 2.0f;

        // Margen para evitar movimientos innecesarios
        float margen = 0.15f;

        // Decidir el movimiento
        if (posBola > centroRaqueta + margen)
        {
            dat->accion = 1; // Subir
        }
        else if (posBola < centroRaqueta - margen)
        {
            dat->accion = -1; // Bajar
        }
        else
        {
            dat->accion = 0; // Quedarse quieto
        }
    }

    // Liberar la memoria compartida
    munmap(dat, sizeof(DatosMemCompartida));

    printf("Partida finalizada. Bot desconectado.\n");

    return 0;
}
