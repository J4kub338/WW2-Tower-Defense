#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>
#include <map>

enum class EnemyType {
    SOLDIER,
    LIGHT_TANK,
    MEDIUM_TANK,
    HEAVY_TANK,
    LIGHT_PLANE,
    MEDIUM_PLANE,
    HEAVY_PLANE
};

enum class Direction {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

class Enemy {
private:
    std::unique_ptr<sf::Sprite> sprite;
    std::map<Direction, std::shared_ptr<sf::Texture>> textures; // store shared_ptr to textures
    EnemyType type;
    std::string name{};          // jawne inicjalizatory, by wyeliminowaæ C26495
    float speed = 0.0f;
    int health = 0;
    int maxHealth = 0;
    int reward = 0; // nagroda za zabicie

    // Œcie¿ka i ruch
    std::vector<sf::Vector2f> path;
    size_t currentWaypoint = 0;
    sf::Vector2f currentPosition{0.0f, 0.0f};
    Direction currentDirection = Direction::WEST;

    // Animacja
    bool isAlive = false;
    float rotationAngle = 0.0f; // dla samolotów

    // Animation-specific
    bool animated = false;           // czy animacja jest aktywna
    int frameCount = 1;              // liczba klatek
    sf::Vector2i frameSize{0,0};     // rozmiar pojedynczej klatki
    int currentFrame = 0;            // indeks aktualnej klatki
    float frameDuration = 0.15f;     // czas trwania klatki (s)
    float frameTimer = 0.0f;         // licznik czasu dla animacji

    // Pasek zdrowia
    sf::RectangleShape healthBarBackground;
    sf::RectangleShape healthBarFill;

public:
    // Added optional textureBaseName parameter for per-map overrides
    Enemy(EnemyType type, const std::vector<sf::Vector2f>& path, int pathIndex = 0, const std::string& textureBaseName = "");
    // £adowanie tekstur (teraz zwraca true jeœli wszystkie pliki za³adowane)
    bool loadTextures(const std::string& baseName);
    void setTextureForDirection(Direction dir);

    // Aktualizacja i rysowanie
    void update(float deltaTime);
    void draw(sf::RenderWindow& window) const;

    // Ruch
    void move(float deltaTime);
    void updateDirection();
    Direction calculateDirection(const sf::Vector2f& from, const sf::Vector2f& to) const;

    // Obra¿enia i zdrowie
    void takeDamage(int damage);
    bool isDead() const { return !isAlive; }

    // Gettery
    sf::Vector2f getPosition() const { return currentPosition; }
    EnemyType getType() const { return type; }
    int getReward() const { return reward; }
    int getHealth() const { return health; }
    float getSpeed() const { return speed; }

    // Konfiguracja wed³ug typu
    void configureByType();

private:
    void updateHealthBar();
    void initializeHealthBar();
};