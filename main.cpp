#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <ctime>

#include <string>

#include <vector>

#define WIDTH 640  // largeur
#define HEIGHT 480 // hauteur

#define APPLE_PATH "../Assets/Apple.png"

#define CELL_SIZE 40

#define QUEUE_SIZE 3

#define SNAKE_WIDTH CELL_SIZE
#define SNAKE_HEIGHT CELL_SIZE

#define MOVE_INTERVAL 0.15f

#define STARTING_X 0
#define STARTING_Y CELL_SIZE * 6

#define MAX_ATTEMPTS (WIDTH / CELL_SIZE) * (HEIGHT / CELL_SIZE)

#define KEY_TO_START SDL_SCANCODE_RETURN    // pour lancer le jeu (menu)
#define KEY_TO_CONTINUE SDL_SCANCODE_RETURN // pour continuer (Dead Screen)
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

// STRUCTS

struct SnakeCell
{
    int gridX = 0, gridY = 0;
    int prevGridX = 0, prevGridY = 0;

    SDL_FRect Frect;
};

// les positions dans les Frect sont purement visuelles
// ce qui compte c'est les gridX && gridY

struct Snake
{
    int gridX = STARTING_X, gridY = STARTING_Y;
    int prevGridX = STARTING_X, prevGridY = STARTING_Y;

    SDL_FRect Frect = {(float)STARTING_X, (float)STARTING_Y, SNAKE_WIDTH, SNAKE_HEIGHT};

    SNAKE_STATES CurrentState = SNAKE_STATES::RIGHT;

    std::vector<SNAKE_STATES> queue;
    std::vector<SnakeCell> SnakeBody;

    float moveCooldown = 0;
};

struct Apple
{
    int gridX = 0;
    int gridY = 0;
    int ID = 0;

    bool eaten = 0;
};

struct Game
{
    GAME_STATES gameState = GAME_STATES::IN_MENU;
    Snake snake;

    std::vector<Apple> apples;

    SDL_Texture *appleTexture = nullptr;

    unsigned int Score = 0;

    float restartCooldown = 0.0f;
};

// DECLARATIONS

void OnAppleIsEaten(Game &game, Apple &apple);
void GrowSnake(Game &game, int amount);
void RemoveApple(Game &game, Apple &_apple);
void PlaceApple(Game &game, int amount);
void ClearGame(Game &game);
void Restart(Game &game);
void HandleSnakeCollision(Game &game);
void HandleGame(Game &game, SDL_Renderer *renderer, float deltaTime);
void drawApples(SDL_Renderer *renderer, Game &game);
void drawSnake(SDL_Renderer *renderer, Snake &snake);
void drawLines(SDL_Renderer *renderer);
void updateSnake(Game &game, float deltaTime);
void renderMenu(SDL_Renderer *renderer);
void renderDeadScreen(SDL_Renderer *renderer);

// HELPERS

inline bool checkGridCollision(int ax, int ay, int bx, int by)
{
    return ax == bx && ay == by;
}

inline float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

// DRAW

void drawLines(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

    for (int x = 0; x <= WIDTH; x += CELL_SIZE)
    {
        SDL_RenderLine(renderer, x, 0, x, HEIGHT);
    }

    for (int y = 0; y <= HEIGHT; y += CELL_SIZE)
    {
        SDL_RenderLine(renderer, 0, y, WIDTH, y);
    }
}

void drawSnake(SDL_Renderer *renderer, Snake &snake)
{
    SDL_SetRenderDrawColor(renderer, 0, 250, 0, 255);
    SDL_RenderFillRect(renderer, &snake.Frect);

    for (auto &snakeCell : snake.SnakeBody)
    {
        SDL_RenderFillRect(renderer, &snakeCell.Frect);
    }
}

void drawApples(SDL_Renderer *renderer, Game &game)
{
    for (auto &apple : game.apples)
    {
        SDL_FRect dest = {(float)apple.gridX, (float)apple.gridY, CELL_SIZE, CELL_SIZE};
        SDL_RenderTexture(renderer, game.appleTexture, NULL, &dest);
    }
}

void drawScore(SDL_Renderer *renderer, Game &game)
{
    std::string Score = "Score: " + std::to_string(game.Score);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, WIDTH / 2, 0 + 0, Score.c_str());
}

// SNAKE

void GrowSnake(Game &game, int amount)
{
    Snake &snake = game.snake;

    for (int i = 0; i < amount; i++)
    {
        int gridX;
        int gridY;

        if (!snake.SnakeBody.empty())
        {
            SnakeCell lastCell = snake.SnakeBody.back();
            gridX = lastCell.gridX;
            gridY = lastCell.gridY;
        }
        else
        {
            gridX = snake.gridX;
            gridY = snake.gridY;
        }

        SnakeCell newCell;
        newCell.gridX = gridX;
        newCell.gridY = gridY;
        newCell.prevGridX = gridX;
        newCell.prevGridY = gridY;
        newCell.Frect = {(float)gridX, (float)gridY, SNAKE_WIDTH, SNAKE_HEIGHT};

        snake.SnakeBody.push_back(newCell);
    }
}

void updateSnake(Game &game, float deltaTime)
{
    Snake &snake = game.snake;

    if (game.gameState != GAME_STATES::IN_GAME)
    {
        snake.moveCooldown = 0;
        return;
    }

    snake.moveCooldown += deltaTime;
    if (snake.moveCooldown > MOVE_INTERVAL)
    {
        snake.moveCooldown -= MOVE_INTERVAL;

        // on sauvegarde les positions avant déplacement
        snake.prevGridX = snake.gridX;
        snake.prevGridY = snake.gridY;

        int prevX = snake.gridX;
        int prevY = snake.gridY;

        if (!snake.queue.empty())
        {
            snake.CurrentState = snake.queue.front();
            snake.queue.erase(snake.queue.begin());
        }

        // on bouge la tête
        switch (snake.CurrentState)
        {
        case SNAKE_STATES::RIGHT:
            snake.gridX += CELL_SIZE;
            break;
        case SNAKE_STATES::LEFT:
            snake.gridX -= CELL_SIZE;
            break;

        case SNAKE_STATES::UP:
            snake.gridY -= CELL_SIZE;
            break;
        case SNAKE_STATES::DOWN:
            snake.gridY += CELL_SIZE;
            break;

        default:
            break;
        }

        for (auto &snakeCell : snake.SnakeBody)
        {
            int currentX = snakeCell.gridX;
            int currentY = snakeCell.gridY;

            snakeCell.prevGridX = currentX;
            snakeCell.prevGridY = currentY;

            snakeCell.gridX = prevX;
            snakeCell.gridY = prevY;

            prevX = currentX;
            prevY = currentY;
        }
    }

    float t = snake.moveCooldown / MOVE_INTERVAL;
    float lerpedX = lerp(snake.prevGridX, snake.gridX, t);
    float lerpedY = lerp(snake.prevGridY, snake.gridY, t);

    snake.Frect.x = lerpedX;
    snake.Frect.y = lerpedY;

    for (auto &snakeCell : snake.SnakeBody)
    {
        snakeCell.Frect.x = lerp(snakeCell.prevGridX, snakeCell.gridX, t);
        snakeCell.Frect.y = lerp(snakeCell.prevGridY, snakeCell.gridY, t);
    }
}

// APPLES

void PlaceApple(Game &game, int amount)
{
    int CellXAmount = WIDTH / CELL_SIZE;
    int CellYAmount = HEIGHT / CELL_SIZE;

    for (int i = 0; i < amount; i++)
    {
        int appleX;
        int appleY;

        int attemps = 0;

        bool positionTaken = false;

        do
        {
            attemps++;
            appleX = (rand() % CellXAmount) * CELL_SIZE;
            appleY = (rand() % CellYAmount) * CELL_SIZE;

            positionTaken = false;

            for (auto &cell : game.snake.SnakeBody)
            {
                if (cell.gridX == appleX && cell.gridY == appleY) // position deja prise par une cell du snake
                {
                    positionTaken = true;
                    break;
                }
            }

            if (!positionTaken)
            {
                for (auto &apple : game.apples)
                {
                    if (apple.gridX == appleX && apple.gridY == appleY) // position deja prise par une autre pomme
                    {
                        positionTaken = true;
                        break;
                    }
                }
            }

            if (!positionTaken)
            {
                if (game.snake.gridX == appleX && game.snake.gridY == appleY) // position deja prise par la tête du snake
                {
                    positionTaken = true;
                }
            }

        } while (positionTaken && attemps < MAX_ATTEMPTS);

        if (!positionTaken)
        {
            int ID = game.apples.size() + 1;
            game.apples.push_back({appleX, appleY, ID});
        }
    }
}

void RemoveApple(Game &game, Apple &_apple)
{
    for (int i = 0; i < game.apples.size(); i++)
    {
        if (game.apples[i].ID == _apple.ID)
        {
            game.apples.erase(game.apples.begin() + i);
            break;
        }
    }
}

void OnAppleIsEaten(Game &game, Apple &apple)
{
    apple.eaten = 1;
    game.Score++;

    RemoveApple(game, apple);

    GrowSnake(game, 1);
    PlaceApple(game, 1);
}

// COLLISIONS

void HandleSnakeCollision(Game &game)
{
    Snake &snake = game.snake;
    // collision sur les murs

    // Verticale
    if (snake.gridY < 0 || snake.gridY >= HEIGHT)
    {
        game.gameState = GAME_STATES::DEAD_SCREEN;
    }

    // Horizontale
    if (snake.gridX < 0 || snake.gridX >= WIDTH)
    {
        game.gameState = GAME_STATES::DEAD_SCREEN;
    }

    for (auto &snakeCell : snake.SnakeBody)
    {
        if (snakeCell.prevGridX == snake.gridX && snakeCell.prevGridY == snakeCell.gridY)
        {
            continue;
        }

        if (checkGridCollision(snake.gridX, snake.gridY, snakeCell.gridX, snakeCell.gridY))
        {
            game.gameState = GAME_STATES::DEAD_SCREEN;
        }
    }

    for (auto &apple : game.apples)
    {
        if (checkGridCollision(snake.gridX, snake.gridY, apple.gridX, apple.gridY) && !apple.eaten)
        {
            OnAppleIsEaten(game, apple);
        }
    }
}

// RENDER

void renderMenu(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, WIDTH / 2, HEIGHT / 2, "MENU");
    SDL_RenderDebugText(renderer, 100, 100, "Snake Game");
    SDL_RenderDebugText(renderer, 200, 200, "Press Enter to start");
}

void renderDeadScreen(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // pour pouvoir voir à travers les pixels

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 175);
    SDL_FRect overlay = {0.0f, 0.0f, WIDTH + 10, HEIGHT + 10};
    SDL_RenderFillRect(renderer, &overlay);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, WIDTH / 2, HEIGHT / 2, "GAME OVER");

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
}

// GAME

void ClearGame(Game &game)
{
    Snake &snake = game.snake;

    if (!snake.queue.empty())
    {
        snake.queue.clear();
    }
    if (!snake.SnakeBody.empty())
    {
        snake.SnakeBody.clear();
    }

    if (!game.apples.empty())
    {
        game.apples.clear();
    }

    game.Score = 0;

    snake.CurrentState = SNAKE_STATES::RIGHT;

    snake.gridX = STARTING_X;
    snake.gridY = STARTING_Y;

    snake.prevGridX = STARTING_X;
    snake.prevGridY = STARTING_Y;

    snake.Frect = {(float)STARTING_X, (float)STARTING_Y, SNAKE_WIDTH, SNAKE_HEIGHT};
}

void Restart(Game &game)
{
    ClearGame(game);
    game.gameState = GAME_STATES::IN_GAME;
    PlaceApple(game, 1);
}

void HandleGame(Game &game, SDL_Renderer *renderer, float deltaTime)
{
    GAME_STATES &gameState = game.gameState;
    Snake &snake = game.snake;

    switch (gameState)
    {
    case GAME_STATES::IN_GAME:

        drawLines(renderer);
        updateSnake(game, deltaTime);
        HandleSnakeCollision(game);
        drawSnake(renderer, snake);
        drawApples(renderer, game);
        drawScore(renderer, game);

        break;

    case GAME_STATES::IN_MENU:
        renderMenu(renderer);

        game.restartCooldown -= deltaTime;
        if (game.restartCooldown < 0)
            game.restartCooldown = 0;

        break;

    case GAME_STATES::DEAD_SCREEN:
        drawLines(renderer);
        drawSnake(renderer, snake);
        renderDeadScreen(renderer);

        game.restartCooldown -= deltaTime;
        if (game.restartCooldown < 0)
            game.restartCooldown = 0;
        break;

    default:
        break;
    }
}

// MAIN

int main(int argc, char *argv[])
{
    srand(time(NULL));

    SDL_Window *window;
    SDL_Renderer *renderer;

    bool running = true;

    if (!SDL_Init(SDL_INIT_VIDEO))
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

    // SDL_Surface *WindowSurface = SDL_GetWindowSurface(window);
    Game game;

    SDL_Surface *surface = IMG_Load(APPLE_PATH);
    game.appleTexture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_DestroySurface(surface);

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

            if (event.type == SDL_EVENT_KEY_DOWN && game.gameState == GAME_STATES::IN_MENU)
            {
                switch (event.key.scancode)
                {
                case KEY_TO_START:
                    if (game.restartCooldown <= 0)
                    {
                        Restart(game);
                        game.restartCooldown = 0.1f;
                    }
                    break;

                default:
                    break;
                }
            }

            else if (event.type == SDL_EVENT_KEY_DOWN && game.gameState == GAME_STATES::DEAD_SCREEN)
            {
                switch (event.key.scancode)
                {
                case KEY_TO_CONTINUE:
                    if (game.restartCooldown <= 0)
                    {
                        Restart(game);
                        game.restartCooldown = 0.1f;
                    }
                    break;

                case KEY_BACK_TO_MENU:
                    if (game.restartCooldown <= 0)
                    {
                        ClearGame(game);
                        game.gameState = GAME_STATES::IN_MENU;
                        game.restartCooldown = 0.1f;
                    }
                    break;
                default:
                    break;
                }
            }

            else if (event.type == SDL_EVENT_KEY_DOWN && game.gameState == GAME_STATES::IN_GAME)
            {
                switch (event.key.scancode)
                {
                case SDL_SCANCODE_UP:
                {
                    SNAKE_STATES last = game.snake.queue.empty() ? game.snake.CurrentState : game.snake.queue.back();

                    // Si la queue est vide alors last = currentState
                    // Sinon on prend le dernier élément de la queue

                    if (game.snake.queue.size() < QUEUE_SIZE && last != SNAKE_STATES::DOWN) // Si la queue n'est pas remplie ET que le dernier élément est différent de DOWN on continue (pour éviter de pouvoir faire des demi-tours)
                        game.snake.queue.push_back(SNAKE_STATES::UP);
                    break;
                }
                case SDL_SCANCODE_DOWN:
                {
                    SNAKE_STATES last = game.snake.queue.empty() ? game.snake.CurrentState : game.snake.queue.back();
                    if (game.snake.queue.size() < QUEUE_SIZE && last != SNAKE_STATES::UP)
                        game.snake.queue.push_back(SNAKE_STATES::DOWN);
                    break;
                }
                case SDL_SCANCODE_LEFT:
                {
                    SNAKE_STATES last = game.snake.queue.empty() ? game.snake.CurrentState : game.snake.queue.back();
                    if (game.snake.queue.size() < QUEUE_SIZE && last != SNAKE_STATES::RIGHT)
                        game.snake.queue.push_back(SNAKE_STATES::LEFT);
                    break;
                }
                case SDL_SCANCODE_RIGHT:
                {
                    SNAKE_STATES last = game.snake.queue.empty() ? game.snake.CurrentState : game.snake.queue.back();
                    if (game.snake.queue.size() < QUEUE_SIZE && last != SNAKE_STATES::LEFT)
                        game.snake.queue.push_back(SNAKE_STATES::RIGHT);
                    break;
                }
                default:
                    break;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // fond noir
        SDL_RenderClear(renderer);

        HandleGame(game, renderer, deltaTime);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(game.appleTexture);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}