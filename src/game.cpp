
#include "game.h"

#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <sstream>
#include <fstream>

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
