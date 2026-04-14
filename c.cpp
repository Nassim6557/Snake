
#include <iostream>
#define Key 0xDEADBEEF

#define SCORE 10

int saveHighScore(int score)
{
    return score ^ Key;
}

int loadHighScore(int score, int checksum)
{
    if ((score ^ Key) == checksum)
    {
        // pas modif
    }
    else
    {
        // modif
    }
}

int main()
{
    int checksum = saveHighScore(SCORE);

    std::cout << checksum << std::endl;

    int s = loadHighScore(SCORE, checksum);

    return 0;
}