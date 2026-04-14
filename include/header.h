#pragma once

#include <SDL3/SDL.h>

#define Key 0xDEADBEEF

#define WIDTH 640  // largeur
#define HEIGHT 480 // hauteur

#define APPLE_PATH "../Assets/Apple.png"
#define FONT_PATH "../Assets/VCR_OSD_MONO_1.ttf"
#define DATA_PATH "../data.dat"

#define CELL_SIZE 40

#define QUEUE_SIZE 3

#define SNAKE_WIDTH CELL_SIZE
#define SNAKE_HEIGHT CELL_SIZE

#define MOVE_INTERVAL 0.15f

#define STARTING_X 0
#define STARTING_Y CELL_SIZE * 6

#define MAX_ATTEMPTS (WIDTH / CELL_SIZE) * (HEIGHT / CELL_SIZE)

#define KEY_TO_START SDL_SCANCODE_RETURN    // pour lancer le jeu (menu)
#define KEY_TO_CONTINUE SDL_SCANCODE_RETURN // pour continuer (écran de mort)
#define KEY_BACK_TO_MENU SDL_SCANCODE_SPACE // pour retourner au menu

// ENUMS

enum class GAME_STATES
{
    IN_MENU,
    IN_GAME,
    DEAD_SCREEN,
};

enum class SNAKE_STATES
{
    RIGHT,
    LEFT,

    UP,
    DOWN
};

// HELPERS

inline bool checkGridCollision(int ax, int ay, int bx, int by)
{
    return ax == bx && ay == by;
}

inline float _lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

// STRUCTS
struct SnakeCell
{
    int gridX = 0, gridY = 0;
    int prevGridX = 0, prevGridY = 0;

    SDL_FRect Frect;
};

struct Apple
{
    int gridX = 0;
    int gridY = 0;
    int ID = 0;
};
