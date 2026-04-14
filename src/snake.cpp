#include "snake.h"

// les positions dans les Frect sont purement visuelles
// ce qui compte c'est les gridX && gridY

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
