# Alumno: Javier Serrano Molina 
# Número de Matrícula: 56613
# Práctica de Laboratorio 3 
# Profesor de Laboratorio: Imanol Izcara 

# Sistemas Informáticos Industriales

Se ha realizado la tercera práctica de laboratorio donde se ha realizado una nueva modificación del programa suministrado basado en el juego Pong, que se describe como "tenis".

## 📖 Descripción

En esta práctica hemos añadido un bot que controla automáticamente la raqueta del jugador 1 en el juego Pong.

Para ello, hemos utilizado memoria compartida mediante mmap(), permitiendo que el juego y el bot intercambien información sobre la posición de la pelota y los movimientos de la raqueta.

Hemos creado DatosMemCompartida.h y bot.cpp, modificado Mundo.cpp y Mundo.h, y actualizado CMakeLists.txt.

Además, hemos mantenido la comunicación FIFO con logger de la práctica anterior y comprobado que los tres programas funcionan correctamente y finalizan al llegar a 3 puntos.
---


