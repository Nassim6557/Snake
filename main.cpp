#include <iostream>
#include <ctime>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <sstream>
#include <fstream>

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

class Game;
class Snake;

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

// les positions dans les Frect sont purement visuelles
// ce qui compte c'est les gridX && gridY

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

void Snake::Tick(float deltaTime)
{
    this->moveCooldown += deltaTime;
    if (this->moveCooldown > MOVE_INTERVAL)
    {
        this->moveCooldown -= MOVE_INTERVAL;

        // on sauvegarde les positions avant déplacement
        this->prevGridX = this->gridX;
        this->prevGridY = this->gridY;

        int prevX = this->gridX;
        int prevY = this->gridY;

        if (!this->queue.empty())
        {
            this->CurrentState = this->queue.front();
            this->queue.erase(this->queue.begin());
        }

        // on déplace la tête
        switch (this->CurrentState)
        {
        case SNAKE_STATES::RIGHT:
            this->gridX += CELL_SIZE;
            break;
        case SNAKE_STATES::LEFT:
            this->gridX -= CELL_SIZE;
            break;

        case SNAKE_STATES::UP:
            this->gridY -= CELL_SIZE;
            break;
        case SNAKE_STATES::DOWN:
            this->gridY += CELL_SIZE;
            break;

        default:
            break;
        }

        for (auto &snakeCell : this->SnakeBody)
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

    float t = this->moveCooldown / MOVE_INTERVAL;
    float lerpedX = _lerp(this->prevGridX, this->gridX, t);
    float lerpedY = _lerp(this->prevGridY, this->gridY, t);

    this->Frect.x = lerpedX;
    this->Frect.y = lerpedY;

    for (auto &snakeCell : this->SnakeBody)
    {
        snakeCell.Frect.x = _lerp(snakeCell.prevGridX, snakeCell.gridX, t);
        snakeCell.Frect.y = _lerp(snakeCell.prevGridY, snakeCell.gridY, t);
    }
}

void Snake::grow(int amount)
{
    for (int i = 0; i < amount; i++)
    {
        int gridX;
        int gridY;

        if (!this->SnakeBody.empty())
        {
            SnakeCell lastCell = this->SnakeBody.back();
            gridX = lastCell.gridX;
            gridY = lastCell.gridY;
        }
        else
        {
            gridX = this->gridX;
            gridY = this->gridY;
        }

        SnakeCell newCell;
        newCell.gridX = gridX;
        newCell.gridY = gridY;
        newCell.prevGridX = gridX;
        newCell.prevGridY = gridY;
        newCell.Frect = {(float)gridX, (float)gridY, SNAKE_WIDTH, SNAKE_HEIGHT};

        this->SnakeBody.push_back(newCell);
    }
}

void Snake::tryEnqueueDirection(SNAKE_STATES newState, SNAKE_STATES opposite)
{
    // Si la queue est vide alors last = currentState
    // Sinon on prend le dernier élément de la queue
    SNAKE_STATES last = this->queue.empty() ? CurrentState : this->queue.back();

    if (this->queue.size() < QUEUE_SIZE && last != opposite) // Si la queue n'est pas remplie ET que le dernier élément est différent de l'opposé on continue (pour éviter de pouvoir faire des demi-tours)
        this->queue.push_back(newState);
}

void Snake::reset()
{
    if (!this->queue.empty())
    {
        this->queue.clear();
    }
    if (!this->SnakeBody.empty())
    {
        this->SnakeBody.clear();
    }

    this->CurrentState = SNAKE_STATES::RIGHT;
    this->moveCooldown = 0;

    this->gridX = STARTING_X;
    this->gridY = STARTING_Y;

    this->prevGridX = STARTING_X;
    this->prevGridY = STARTING_Y;

    this->Frect = {(float)STARTING_X, (float)STARTING_Y, SNAKE_WIDTH, SNAKE_HEIGHT};
}

// Collisions du Snake
bool Snake::CheckWallCollision()
{
    // Verticale

    if (this->gridY < 0 || this->gridY >= HEIGHT)
    {
        return true;
    }

    // Horizontale
    if (this->gridX < 0 || this->gridX >= WIDTH)
    {
        return true;
    }

    return false;
}

bool Snake::CheckSelfCollision()
{
    for (auto &snakeCell : this->SnakeBody)
    {
        if (snakeCell.prevGridX == this->gridX && snakeCell.prevGridY == this->gridY)
        {
            continue;
        }

        if (checkGridCollision(this->gridX, this->gridY, snakeCell.gridX, snakeCell.gridY))
        {
            return true;
        }
    }

    return false;
}

Apple *Snake::CheckAppleCollision(std::vector<Apple> &apples)
{
    for (auto &apple : apples)
    {
        if (checkGridCollision(this->gridX, this->gridY, apple.gridX, apple.gridY))
        {
            return &apple;
        }
    }

    return nullptr;
}

// LOGIQUE DU JEU

bool Game::Init(SDL_Renderer *renderer)
{

    this->Bigfont = TTF_OpenFont(FONT_PATH, 30.0f);
    this->Basicfont = TTF_OpenFont(FONT_PATH, 22.0f);
    this->Smallfont = TTF_OpenFont(FONT_PATH, 15.0f);

    if (!this->Bigfont)
    {
        SDL_Log("Failed to load font: %s", SDL_GetError());
        return false;
    }
    if (!this->Basicfont)
    {
        SDL_Log("Failed to load font: %s", SDL_GetError());
        return false;
    }
    if (!this->Smallfont)
    {
        SDL_Log("Failed to load font: %s", SDL_GetError());
        return false;
    }

    SDL_Surface *surface = IMG_Load(APPLE_PATH);

    if (!surface)
    {
        return false;
    }

    this->appleTexture = SDL_CreateTextureFromSurface(renderer, surface);

    SDL_DestroySurface(surface);

    this->loadHighScore();
    return true;
}

void Game::Shutdown()
{
    this->saveHighScore();
    TTF_CloseFont(Basicfont);
    TTF_CloseFont(Bigfont);
    TTF_CloseFont(Smallfont);
    SDL_DestroyTexture(appleTexture);
}

void Game::UpdateCooldown(float deltaTime)
{
    this->restartCooldown -= deltaTime;
    if (this->restartCooldown < 0)
        this->restartCooldown = 0;
}

void Game::InGameUpdate(float deltaTime)
{
    snake.Tick(deltaTime); // on déplace le snake

    if (snake.CheckWallCollision() || snake.CheckSelfCollision())
    {
        setGameState(GAME_STATES::DEAD_SCREEN);
        return;
    }

    Apple *eaten = snake.CheckAppleCollision(this->apples);
    if (eaten)
        OnAppleIsEaten(*eaten);
}

void Game::RemoveApple(Apple &_apple)
{
    for (int i = 0; i < this->apples.size(); i++)
    {
        if (this->apples[i].ID == _apple.ID)
        {
            this->apples.erase(this->apples.begin() + i);
            break;
        }
    }
}

void Game::render(SDL_Renderer *renderer)
{
    drawLines(renderer);
    drawSnake(renderer);
    drawApples(renderer);
    drawScore(renderer);
}

void Game::PlaceApple(int amount)
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

            for (auto &cell : this->snake.getSnakeBody())
            {
                if (cell.gridX == appleX && cell.gridY == appleY) // position déjà prise par une cellule du snake
                {
                    positionTaken = true;
                    break;
                }
            }

            if (!positionTaken)
            {
                for (auto &apple : this->apples)
                {
                    if (apple.gridX == appleX && apple.gridY == appleY) // position déjà prise par une autre pomme
                    {
                        positionTaken = true;
                        break;
                    }
                }
            }

            if (!positionTaken)
            {
                if (this->snake.getGridX() == appleX && this->snake.getGridY() == appleY) // position déjà prise par la tête du snake
                {
                    positionTaken = true;
                }
            }

        } while (positionTaken && attemps < MAX_ATTEMPTS);

        if (!positionTaken)
        {
            int ID = this->apples.size() + 1;
            this->apples.push_back({appleX, appleY, ID});
        }
    }
}

void Game::Handle(SDL_Renderer *renderer, float deltaTime)
{
    switch (this->gameState)
    {
    case GAME_STATES::IN_GAME:
        this->InGameUpdate(deltaTime);
        this->render(renderer);

        break;

    case GAME_STATES::IN_MENU:
        this->renderMenu(renderer);
        this->UpdateCooldown(deltaTime);
        break;

    case GAME_STATES::DEAD_SCREEN:

        this->drawLines(renderer);
        this->drawSnake(renderer);

        this->renderDeadScreen(renderer);
        this->UpdateCooldown(deltaTime);

        break;

    default:
        break;
    }
}

void Game::OnAppleIsEaten(Apple &apple)
{
    this->increaseScore();

    this->RemoveApple(apple);
    this->PlaceApple(1);

    this->snake.grow(1);
}

void Game::ClearGame()
{
    snake.reset();

    if (!this->apples.empty())
    {
        this->apples.clear();
    }

    this->Score = 0;
}

void Game::Restart()
{
    this->ClearGame();
    this->gameState = GAME_STATES::IN_GAME;
    this->PlaceApple(1);
}

// SCORE
/*

Score = 5
Key = 3

-> Sauvegarde
On va sauvegarder le score et le checksum

    Score   =       101     -    5
    Key     =       011     -    3
    ^               XOR
    CheckSum =      110     -    6

-> Chargement
    On a : Score & CheckSum

    101
    011
    XOR
    CheckSum

    Si      Score XOR Key == CheckSum
        on est sûr que le score n'a pas été modifié
*/

bool Game::saveHighScore()
{
    unsigned short int current_maxScore = this->MaxScore;
    int checksum = current_maxScore ^ Key;

    std::ofstream fileOut(DATA_PATH);
    if (!fileOut)
        return false;

    fileOut << current_maxScore << " " << checksum;
    fileOut.close();

    return true;
}

bool Game::loadHighScore()
{
    std::ifstream dataFile(DATA_PATH); // pour lire dans le fichier

    std::string data;
    getline(dataFile, data);

    int datascore = 0, datachecknum = 0;

    if (!data.empty())
    {
        std::istringstream iss(data);
        iss >> datascore >> datachecknum; // extraire les 2 premiers int de la ligne
    }

    if ((datascore ^ Key) == datachecknum)
    {
        this->MaxScore = datascore;
    }
    else
    {
        // modification

        this->MaxScore = 0;

        std::ofstream fileOut(DATA_PATH); // vide le fichier
        fileOut.close();
    }

    dataFile.close();
    return true;
}

// AFFICHAGE
bool RenderLabel(SDL_Renderer *renderer, TTF_Font *font, std::string text, SDL_Color color, float x, float y)
{
    SDL_Surface *Surface = TTF_RenderText_Blended(font, text.c_str(), text.size(), color);

    if (!Surface)
    {
        std::cerr << "TTF_RenderText_Blended error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Texture *Texture = SDL_CreateTextureFromSurface(renderer, Surface);
    SDL_DestroySurface(Surface);

    if (!Texture)
    {
        std::cerr << "SDL_CreateTextureFromSurface error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_FRect Frect = {(x) - (float)Texture->w / 2, (y) - (float)Texture->h / 2, (float)Texture->w, (float)Texture->h};

    bool result = SDL_RenderTexture(renderer, Texture, NULL, &Frect);

    return result;
}

void Game::drawLines(SDL_Renderer *renderer)
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

void Game::drawSnake(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 0, 250, 0, 255);
    SDL_RenderFillRect(renderer, &this->snake.Frect);

    for (auto &snakeCell : this->snake.getSnakeBody())
    {
        SDL_RenderFillRect(renderer, &snakeCell.Frect);
    }
}

void Game::drawApples(SDL_Renderer *renderer)
{
    for (auto &apple : this->getApples())
    {
        SDL_FRect dest = {(float)apple.gridX, (float)apple.gridY, CELL_SIZE, CELL_SIZE};
        SDL_RenderTexture(renderer, this->appleTexture, NULL, &dest);
    }
}

void Game::drawScore(SDL_Renderer *renderer)
{
    std::string Score = "Score: " + std::to_string(this->Score);

    SDL_Color color = {255, 255, 255, 255};

    if (this->Score >= this->MaxScore)
    {
        color = {255, 255, 0, 255};
    }

    RenderLabel(renderer, this->Basicfont, Score, color, WIDTH / 2.0f, 30.0f);
}

// RENDU

void Game::renderDeadScreen(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // pour pouvoir voir à travers les pixels

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 175);
    SDL_FRect overlay = {0.0f, 0.0f, WIDTH + 10, HEIGHT + 10};
    SDL_RenderFillRect(renderer, &overlay);

    SDL_Color green = {74, 222, 128, 255};
    SDL_Color red = {255, 0, 0, 255};
    SDL_Color white = {255, 255, 255, 255};

    RenderLabel(renderer, this->Bigfont, "GAME OVER", red,
                WIDTH / 2.0f, HEIGHT / 2.0f - 120.0f);

    auto maxScore = this->MaxScore;
    auto Score = this->Score;

    std::string Scorestr = "Score: " + std::to_string(Score);
    std::string MaxScorestr = "Max: " + std::to_string(maxScore);

    std::string str = Scorestr + "   " + MaxScorestr;

    RenderLabel(renderer, this->Smallfont, str, white,
                WIDTH / 2.0f, HEIGHT / 2.0f);

    RenderLabel(renderer, this->Smallfont, "PRESS SPACE TO RETURN TO MENU", white,
                WIDTH / 2.0f, HEIGHT - 50.0f);
    RenderLabel(renderer, this->Smallfont, "PRESS ENTER TO CONTINUE", white,
                WIDTH / 2.0f, HEIGHT - 80.0f);
}

void Game::renderMenu(SDL_Renderer *renderer)
{
    SDL_SetRenderDrawColor(renderer, 10, 10, 10, 255);

    SDL_Color green = {74, 222, 128, 255};
    SDL_Color greenDark = {22, 163, 74, 255};
    SDL_Color grey = {255, 255, 255, 80};
    SDL_Color white = {255, 255, 255, 255};

    RenderLabel(renderer, this->Bigfont, "SNAKE", green,
                WIDTH / 2.0f, HEIGHT / 2.0f - 120.0f);

    SDL_SetRenderDrawColor(renderer, 22, 163, 74, 255);
    SDL_FRect line = {WIDTH / 2.0f - 40, HEIGHT / 2.0f - 95, 80, 2};
    SDL_RenderFillRect(renderer, &line);

    auto maxScore = this->getMaxScore();
    std::string maxScorestr = "Max Score: " + std::to_string(maxScore);

    RenderLabel(renderer, this->Smallfont, maxScorestr, white,
                WIDTH / 2.0f, HEIGHT / 2.0f);

    RenderLabel(renderer, this->Smallfont, "PRESS ENTER TO PLAY", grey,
                WIDTH / 2.0f, HEIGHT - 50.0f);
}

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