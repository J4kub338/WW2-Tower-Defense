#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <map>
#include <memory>

using namespace sf;
using namespace std;

enum class TowerType {
    INFANTRY_POST,  // Stanowisko piechoty
    MACHINE_GUN,     // Karabin maszynowy
    ARTILLERY,       // Artyleria
    AA_GUN          // Dzia³o przeciwlotnicze  
};

struct TowerConfig {
    std::string name;
    float range;        // Zasiêg (piksele)
    int damage;         // Obra¿enia
    float fireRate;     // Strza³y na sekundê
    int cost;           // Koszt budowy
    std::string textureFile; // Œcie¿ka do tekstury
    int upgradeCost;    // Koszt ulepszenia
    sf::Vector2f size;  // Rozmiar sprite (px)
};

class Tower {
private:
    std::unique_ptr<sf::Sprite> sprite; // Wymagany wskaŸnik w SFML 3.0
    const sf::Texture* texturePtr; // wskaznik do zewnêtrznej tekstury (nie kopiujemy)
    sf::CircleShape rangeCircle;
    TowerType type;
    TowerConfig config;
    int level;
    bool showRange;
    sf::Vector2f centerPos; // przechowuje œrodek wie¿y (w pikselach)

    // firing state
    float cooldownTimer = 0.0f; // seconds until next shot

public:
    // Konstruktor - nie tworzy ju¿ sprite/texture (to robi Game.cpp)
    Tower(float x, float y, TowerType type = TowerType::MACHINE_GUN);

    // Metody
    void draw(sf::RenderWindow& window) const;
    void setPosition(float x, float y);
    void toggleRangeDisplay(bool show);

    // Ustawia teksturê i tworzy sprite wewn¹trz wie¿y
    void setTexture(const sf::Texture& tex);

    // rotate tower to face given direction vector (world-space)
    void rotateToDirection(const sf::Vector2f& dir);

    // cooldown management
    void tickCooldown(float dt);
    bool canFire() const;
    void resetCooldown();

    // Gettery
    sf::Vector2f getPosition() const;
    sf::FloatRect getBounds() const;
    float getRange() const;
    int getDamage() const;
    TowerType getType() const;
    const std::string& getName() const;
    int getCost() const;
    int getUpgradeCost() const;
    std::string getTexture() const;

    // Logika
    void upgrade();

private:
    void updateVisuals();
};

// Konfiguracje wie¿ (definicja w .cpp)
extern const std::map<TowerType, TowerConfig> TOWER_CONFIGS;