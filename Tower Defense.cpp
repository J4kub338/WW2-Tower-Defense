#include <iostream>
#include <SFML/Graphics.hpp>
#include "Game.hpp"

int main()
{
    // Gra zarządza oknem i menu wewnątrz siebie
    Game game;
    game.run();
    return 0;
}