#pragma once
#include <SFML/Graphics.hpp>
#include "Map.hpp"
#include "Tower.hpp"
#include "WaveManager.hpp"
#include <vector>
#include <unordered_map>
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Font.hpp"
#include "Projectile.hpp"
#include "Sounds.hpp"


using namespace sf;
using namespace std;   

class Game {
private:
    sf::RenderWindow window;
    Map map;
    std::vector<Tower> towers;
    std::vector<std::unique_ptr<Enemy>> enemies;
    WaveManager waveManager;
    TowerType selectedTowerType;
    bool showAllRanges;
    bool isRunning;
    std::unordered_map<TowerType, sf::Texture> towerTextures;
    std::unordered_map<TowerType, sf::Texture> bulletTextures;
    int PlayerHP = 100;
    sf::Font font;
    Font fontForText;
    Text HPText;
    Text CoinsText;
	int PlayerCoins = 300;
    Texture hudTexture;
    std::unique_ptr<Sprite> hudSprite; // SFML 3 Sprite has no default ctor

    std::vector<std::unique_ptr<Projectile>> projectiles;


    //Sounds
	SoundManager soundManager;

    // UI for testing waves (no text to avoid SFML font API differences)
    sf::RectangleShape startButton;
    sf::RectangleShape stopButton;

    // Low-coins message
    Text lowCoinsText;
    bool showLowCoinsMessage = false;
    float lowCoinsMessageTimer = 0.0f; // seconds

    // Metody prywatne
    void processEvents();
    void update(float deltaTime);
    void render();
    void handleMouseClick(const sf::Vector2f& position);
    void HPTextUpdate();

public:
    Game();
    void run();
};