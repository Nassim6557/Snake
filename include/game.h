#pragma once

#include "header.h"
#include <vector>
#include <SDL3_ttf/SDL_ttf.h>

#include "snake.h"

class Game
{
public:
    Game() {};

    float restartCooldown = 0.0f;

    bool Init(SDL_Renderer *renderer);
    void Shutdown();

    void PlaceApple(int amount);
    void RemoveApple(Apple &_apple);

    void ClearGame();
    void Restart();
    void render(SDL_Renderer *renderer);
    void Handle(SDL_Renderer *renderer, float deltaTime);
    void OnAppleIsEaten(Apple &apple);

    bool saveHighScore();
    bool loadHighScore();

    void InGameUpdate(float deltaTime);

    void UpdateCooldown(float deltaTime);

    Snake &getSnake() { return snake; }
    const Snake &getSnake() const { return snake; }

    std::vector<Apple> &getApples() { return apples; }
    const std::vector<Apple> &getApples() const { return apples; }

    unsigned short int getScore() const { return Score; }
    unsigned short int getMaxScore() const { return MaxScore; }
    GAME_STATES getGameState() { return gameState; };

    void drawScore(SDL_Renderer *renderer);
    void drawApples(SDL_Renderer *renderer);
    void drawSnake(SDL_Renderer *renderer);
    void drawLines(SDL_Renderer *renderer);

    void renderMenu(SDL_Renderer *renderer);
    void renderDeadScreen(SDL_Renderer *renderer);

    void setGameState(GAME_STATES s)
    {
        if (this->gameState != s)
            this->gameState = s;
    };

    void resetMaxScore()
    {
        this->MaxScore = 0;
    }

    void increaseScore()
    {
        this->Score++;
        if (this->Score > this->MaxScore)
            this->MaxScore = this->Score;
    };

private:
    Snake snake; // création du snake
    GAME_STATES gameState = GAME_STATES::IN_MENU;

    TTF_Font *Basicfont;
    TTF_Font *Smallfont;
    TTF_Font *Bigfont;

    SDL_Texture *appleTexture = nullptr;

    std::vector<Apple> apples;

    unsigned short int Score = 0; // min : 0
                                  // max : 32 767
    unsigned short int MaxScore = 0;
};
