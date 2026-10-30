CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(TENIS)

# Directorio de archivos de cabecera
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)

# Archivos utilizados por el juego
SET(COMMON_SRCS
    src/Mundo.cpp
    src/Esfera.cpp
    src/Plano.cpp
    src/Raqueta.cpp
    src/Vector2D.cpp
)

# Programa principal
ADD_EXECUTABLE(tenis src/tenis.cpp ${COMMON_SRCS})

# Programa de comunicacion FIFO
ADD_EXECUTABLE(logger src/logger.cpp)

# Programa de memoria compartida
ADD_EXECUTABLE(bot src/bot.cpp)

# Librerias graficas del juego
TARGET_LINK_LIBRARIES(tenis glut GL GLU)
