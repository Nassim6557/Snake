#pragma once

#include <vector>
#include "header.h"

class Snake
{
public:
    Snake() {};

    void Tick(float deltaTime);
    void grow(int amount);

    bool CheckWallCollision(); // true = mort
    bool CheckSelfCollision();
    Apple *CheckAppleCollision(std::vector<Apple> &apples);

    void tryEnqueueDirection(SNAKE_STATES newState, SNAKE_STATES opposite);

    int getGridX() const { return gridX; }
    int getGridY() const { return gridY; }

    void reset();

    SDL_FRect Frect = {(float)STARTING_X, (float)STARTING_Y, SNAKE_WIDTH, SNAKE_HEIGHT};

    std::vector<SnakeCell> &getSnakeBody() { return this->SnakeBody; };
    std::vector<SNAKE_STATES> &getQueue() { return this->queue; };

    SNAKE_STATES &getCurentState() { return this->CurrentState; };

private:
    float moveCooldown = 0;
    int gridX = STARTING_X, gridY = STARTING_Y;
    int prevGridX = STARTING_X, prevGridY = STARTING_Y;

    SNAKE_STATES CurrentState = SNAKE_STATES::RIGHT;

    std::vector<SNAKE_STATES> queue;
    std::vector<SnakeCell> SnakeBody;
};