#include "Enemy.hpp"
#include <iostream>
#include <cmath>

// Konfiguracja dla typów przeciwników
namespace {
    struct EnemyConfig {
        std::string name;
        float speed;
        int health;
        int reward;
        std::string textureName; // base unit name, without map prefix (e.g. "light_tank")
    };

    // Map each enemy type to the correct base filename (unit name only, no map prefix)
    const std::map<EnemyType, EnemyConfig> ENEMY_CONFIGS = {
        {EnemyType::SOLDIER,     {"Soldier",      70.0f, 120, 5, "soldier"}}, //70 basic speed
        {EnemyType::LIGHT_TANK,  {"Light Tank",   80.0f, 750, 20, "light_tank"}},
        {EnemyType::MEDIUM_TANK, {"Medium Tank",  70.0f,1100, 40, "med_tank"}},
        {EnemyType::HEAVY_TANK,  {"Heavy Tank",   55.0f,1500, 60, "heavy_tank"}},
        {EnemyType::LIGHT_PLANE, {"Light Plane",  90.0f, 500, 20, "light_plane"}},
        {EnemyType::MEDIUM_PLANE,{"Medium Plane", 80.0f, 750, 30, "med_plane"}},
        {EnemyType::HEAVY_PLANE, {"Heavy Plane",  70.0f, 1250,50, "heavy_plane"}},
		{EnemyType::MINI_BOSS,   {"Mini Boss",    30.0f, 7500, 450, "mini_boss"}},
        {EnemyType::BOSS, {"Boss", 25.0f, 18000, 1000, "boss"}}
    };
}

Enemy::Enemy(EnemyType type, const std::vector<sf::Vector2f>& path, int pathIndex, const std::string& textureBaseName)
    : type(type),
      path(path),
      currentWaypoint(1), // zaczynamy od pierwszego waypointa (0 to start)
      isAlive(true),
      rotationAngle(0.0f),
      currentDirection(Direction::WEST),
      speed(0.0f),
      health(0),
      maxHealth(0),
      reward(0)
{
    if (path.empty()) {
        std::cerr << "Error: Enemy created with empty path!\n";
        return;
    }

    // Ustaw pozycjê startow¹
    currentPosition = path[0];

    // Skonfiguruj wed³ug typu
    configureByType();

    // Build final texture base name. If a map prefix was provided, prefix it (e.g. "GER" + "_" + "light_tank" => "GER_light_tank").
    const auto& cfg = ENEMY_CONFIGS.at(type);
    std::string baseNameToUse;
    if (!textureBaseName.empty()) {
        baseNameToUse = textureBaseName + "_" + cfg.textureName;
    } else {
        baseNameToUse = cfg.textureName; // fallback: try unit name alone
    }

    // Load textures using the composed base name
    loadTextures(baseNameToUse);

    // Ustaw pocz¹tkowy kierunek i teksturê
    if (path.size() > 1) {
        currentDirection = calculateDirection(path[0], path[1]);
        setTextureForDirection(currentDirection);
    }
    else {
        currentDirection = Direction::SOUTH;
        setTextureForDirection(currentDirection);
    }

    // Ustaw origin i pozycjê sprite (jeœli sprite zosta³ utworzony)
    if (sprite) {
        // If animated, origin should be half frameSize, otherwise local bounds
        if (animated && frameSize.x > 0 && frameSize.y > 0) {
            sprite->setOrigin({ frameSize.x / 2.0f, frameSize.y / 2.0f });
        } else {
            sf::FloatRect bounds = sprite->getLocalBounds();
            sprite->setOrigin({ bounds.size.x / 2.0f, bounds.size.y / 2.0f });
        }
        sprite->setPosition(currentPosition);
    }

    // Inicjalizacja paska zdrowia
    initializeHealthBar();
}

void Enemy::configureByType() {
    const auto& config = ENEMY_CONFIGS.at(type);
    name = config.name;
    speed = config.speed;
    maxHealth = config.health;
    health = maxHealth;
    reward = config.reward;

    // Default animation settings
    animated = false;
    frameCount = 1;
    frameSize = {0,0};
    currentFrame = 0;
    frameDuration = 0.15f;
    frameTimer = 0.0f;

    if (type == EnemyType::SOLDIER) {
        // soldier uses 4-frame horizontal spritesheet
        animated = true;
        frameCount = 4;
        frameDuration = 0.12f; // faster walking animation
    }
}

bool Enemy::loadTextures(const std::string& baseName) {
    // First try single-file texture (e.g. GER_light_tank.png or light_tank.png)
    {
        auto tex = std::make_shared<sf::Texture>();
        std::string singlePath = "assets/enemies/" + baseName + ".png";
        if (tex->loadFromFile(singlePath)) {
            for (auto d : {Direction::NORTH, Direction::EAST, Direction::SOUTH, Direction::WEST}) {
                textures[d] = tex;
            }
            // compute frameSize for soldier if needed
            if (animated && type == EnemyType::SOLDIER) {
                sf::Vector2u sz = tex->getSize();
                if (frameCount > 0) {
                    frameSize.x = static_cast<int>(sz.x / frameCount);
                    frameSize.y = static_cast<int>(sz.y);
                }
            }
            return true;
        }
    }

    // If single file not found, try directional variants (e.g. GER_light_tank_NORTH.png)
    std::map<Direction, std::string> dirNames = {
        {Direction::NORTH, baseName + "_NORTH.png"},
        {Direction::EAST,  baseName + "_EAST.png"},
        {Direction::SOUTH, baseName + "_SOUTH.png"},
        {Direction::WEST,  baseName + "_WEST.png"}
    };

    bool anyLoaded = false;
    std::map<Direction, std::shared_ptr<sf::Texture>> loaded;

    for (const auto& kv : dirNames) {
        Direction dir = kv.first;
        const std::string filename = kv.second;
        auto tex = std::make_shared<sf::Texture>();
        if (tex->loadFromFile("assets/enemies/" + filename)) {
            loaded[dir] = tex;
            anyLoaded = true;
        }
    }

    if (anyLoaded) {
        // If some directional textures loaded, use them. For missing directions, reuse any available texture.
        std::shared_ptr<sf::Texture> anyTex = nullptr;
        for (const auto& kv : loaded) { if (kv.second) { anyTex = kv.second; break; } }
        for (auto d : {Direction::NORTH, Direction::EAST, Direction::SOUTH, Direction::WEST}) {
            if (loaded.count(d)) textures[d] = loaded[d];
            else textures[d] = anyTex; // fallback
        }

        // If soldier animation, compute frameSize from one of the loaded textures
        if (animated && type == EnemyType::SOLDIER && anyTex) {
            sf::Vector2u sz = anyTex->getSize();
            if (frameCount > 0) {
                frameSize.x = static_cast<int>(sz.x / frameCount);
                frameSize.y = static_cast<int>(sz.y);
            }
        }

        return true;
    }

    std::cerr << "Nie mozna zaladowac tekstur dla: " << baseName << std::endl;
    return false;
}

void Enemy::setTextureForDirection(Direction dir) {
    auto it = textures.find(dir);
    if (it != textures.end() && it->second) {
        if (!sprite) {
            sprite = std::make_unique<sf::Sprite>(*it->second);
        } else {
            sprite->setTexture(*it->second);
        }

        if (animated && frameSize.x > 0 && frameSize.y > 0) {
            int left = (currentFrame % frameCount) * frameSize.x;
            sprite->setTextureRect(sf::IntRect({ left, 0 }, { frameSize.x, frameSize.y }));
            sprite->setOrigin({ frameSize.x / 2.0f, frameSize.y / 2.0f });
        } else {
            // Dla samolotów i czo³gów obracamy sprite
            if (type == EnemyType::LIGHT_PLANE ||
                type == EnemyType::MEDIUM_PLANE ||
                type == EnemyType::HEAVY_PLANE ||
                type == EnemyType::LIGHT_TANK ||
                type == EnemyType::MEDIUM_TANK ||
                type == EnemyType::HEAVY_TANK ||
				type == EnemyType::MINI_BOSS ||
                type == EnemyType::BOSS) {

                float rotation = 0.0f;
                switch (dir) {
                case Direction::NORTH: rotation = 0.0f; break;
                case Direction::EAST: rotation = 90.0f; break;
                case Direction::SOUTH: rotation = 180.0f; break;
                case Direction::WEST: rotation = 270.0f; break;
                }
                if (sprite) sprite->setRotation(sf::degrees(rotation));
            }

            if (sprite) {
                sf::FloatRect bounds = sprite->getLocalBounds();
                sprite->setOrigin({ bounds.size.x / 2.0f, bounds.size.y / 2.0f });
            }
        }

        // If sprite was just created, set its position
        if (sprite) {
            sprite->setPosition(currentPosition);
        }
    }
}

Direction Enemy::calculateDirection(const sf::Vector2f& from, const sf::Vector2f& to) const {
    float dx = to.x - from.x;
    float dy = to.y - from.y;

    // Ustalamy g³ówny kierunek na podstawie wiêkszej ró¿nicy
    if (std::abs(dx) > std::abs(dy)) {
        return (dx > 0) ? Direction::EAST : Direction::WEST;
    }
    else {
        return (dy > 0) ? Direction::SOUTH : Direction::NORTH;
    }
}

void Enemy::updateDirection() {
    if (currentWaypoint >= path.size()) {
        return; // Dotar³ do koñca
    }

    const sf::Vector2f& target = path[currentWaypoint];
    Direction newDirection = calculateDirection(currentPosition, target);

    if (newDirection != currentDirection) {
        currentDirection = newDirection;
        currentFrame = 0; // reset walking animation on direction change
        setTextureForDirection(currentDirection);
    }
}

void Enemy::move(float deltaTime) {
    if (currentWaypoint >= path.size()) {
        // Dotar³ do koñca œcie¿ki - uciek³
        isAlive = false;
        return;
    }

    const sf::Vector2f& target = path[currentWaypoint];
    sf::Vector2f direction = target - currentPosition;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance < 1.0f) {
        // Dotar³ do waypointa
        currentWaypoint++;
        if (currentWaypoint < path.size()) {
            updateDirection();
        }
        return;
    }

    // Normalizuj kierunek i przesuñ
    direction /= distance;
    currentPosition += direction * speed * deltaTime;

    // Aktualizuj pozycjê sprite
    if (sprite) sprite->setPosition(currentPosition);

    // Aktualizuj pasek zdrowia
    updateHealthBar();
}

void Enemy::update(float deltaTime) {
    if (!isAlive) return;

    // advance animation if applicable
    if (animated && sprite) {
        frameTimer += deltaTime;
        while (frameTimer >= frameDuration) {
            frameTimer -= frameDuration;
            currentFrame = (currentFrame + 1) % frameCount;
            int left = (currentFrame % frameCount) * frameSize.x;
            sprite->setTextureRect(sf::IntRect({ left, 0 }, { frameSize.x, frameSize.y }));
        }
    }

    move(deltaTime);
}

void Enemy::draw(sf::RenderWindow& window) const {
    if (!isAlive) return;

    if (sprite) window.draw(*sprite);

    // Rysuj pasek zdrowia (tylko jeœli nie ma pe³nego HP)
    if (health < maxHealth) {
        window.draw(healthBarBackground);
        window.draw(healthBarFill);
    }
}

void Enemy::initializeHealthBar() {
    sf::FloatRect bounds;
    if (sprite) bounds = sprite->getLocalBounds();
    else bounds = { {0,0}, {32.0f, 32.0f} };

    float width = bounds.size.x * 0.8f;
    float height = 5.0f;

    healthBarBackground.setSize({ width, height });
    healthBarBackground.setFillColor(sf::Color::Black);
    healthBarBackground.setOutlineThickness(1.0f);
    healthBarBackground.setOutlineColor(sf::Color::White);

    healthBarFill.setSize({ width, height });
    healthBarFill.setFillColor(sf::Color::Red);

    updateHealthBar();
}

void Enemy::updateHealthBar() {
    sf::FloatRect bounds = { {0,0}, {0,0} };
    if (sprite) bounds = sprite->getGlobalBounds();

    float barY = bounds.position.y - 15.0f;
    float barX = currentPosition.x - healthBarBackground.getSize().x / 2.0f;

    healthBarBackground.setPosition({ barX, barY });
    healthBarFill.setPosition({ barX, barY });

    // Aktualizuj szerokoœæ paska HP
    float healthPercentage = (maxHealth > 0) ? static_cast<float>(health) / maxHealth : 0.0f;
    healthBarFill.setSize({
        healthBarBackground.getSize().x * healthPercentage,
        healthBarFill.getSize().y
        });
}

void Enemy::takeDamage(int damage) {
    health -= damage;
    if (health <= 0) {
        health = 0;
        isAlive = false;
    }
    updateHealthBar();
}