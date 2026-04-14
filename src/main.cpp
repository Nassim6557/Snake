#include <iostream>
#include <ctime>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "header.h"

#include "game.h"
#include "snake.h"

// MAIN

int main(int argc, char *argv[])
{
    srand(time(NULL));

    SDL_Window *window;
    SDL_Renderer *renderer;

    bool running = true;

    if (!SDL_Init(SDL_INIT_VIDEO) || !TTF_Init())
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Init Error: %s\n", SDL_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Snake", WIDTH, HEIGHT, 0);
    if (window == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (renderer == NULL)
    {
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create renderer: %s\n", SDL_GetError());
        return 1;
    }

    SDL_SetRenderVSync(renderer, 1);

    Game game;

    if (!game.Init(renderer))
    {
        return -1;
    }

    Uint64 currentTime = SDL_GetTicks();
    Uint64 previousTime = currentTime;
    float deltaTime = 0.0f;

    while (running)
    {
        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN && game.getGameState() == GAME_STATES::IN_MENU)
            {
                switch (event.key.scancode)
                {
                case KEY_TO_START:
                    if (game.restartCooldown <= 0)
                    {
                        game.Restart();
                        game.restartCooldown = 0.1f;
                    }
                    break;

                default:
                    break;
                }
            }

            else if (event.type == SDL_EVENT_KEY_DOWN && game.getGameState() == GAME_STATES::DEAD_SCREEN)
            {
                switch (event.key.scancode)
                {
                case KEY_TO_CONTINUE:
                    if (game.restartCooldown <= 0)
                    {
                        game.Restart();
                        game.restartCooldown = 0.1f;
                    }
                    break;

                case KEY_BACK_TO_MENU:
                    if (game.restartCooldown <= 0)
                    {
                        game.ClearGame();
                        game.setGameState(GAME_STATES::IN_MENU);
                        game.restartCooldown = 0.1f;
                    }
                    break;
                default:
                    break;
                }
            }

            else if (event.type == SDL_EVENT_KEY_DOWN && game.getGameState() == GAME_STATES::IN_GAME)
            {
                switch (event.key.scancode)
                {
                case SDL_SCANCODE_UP:
                    game.getSnake().tryEnqueueDirection(SNAKE_STATES::UP, SNAKE_STATES::DOWN);
                    break;
                case SDL_SCANCODE_DOWN:
                    game.getSnake().tryEnqueueDirection(SNAKE_STATES::DOWN, SNAKE_STATES::UP);
                    break;
                case SDL_SCANCODE_LEFT:
                    game.getSnake().tryEnqueueDirection(SNAKE_STATES::LEFT, SNAKE_STATES::RIGHT);
                    break;
                case SDL_SCANCODE_RIGHT:
                    game.getSnake().tryEnqueueDirection(SNAKE_STATES::RIGHT, SNAKE_STATES::LEFT);
                    break;
                default:
                    break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // fond noir
        SDL_RenderClear(renderer);

        game.Handle(renderer, deltaTime);

        SDL_RenderPresent(renderer);
    }

    game.Shutdown();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    TTF_Quit();
    SDL_Quit();
    return 0;
}