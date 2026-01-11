#include "Tower.hpp"
#include <iostream>
#include <cmath>

// Konfiguracje wie¿
const std::map<TowerType, TowerConfig> TOWER_CONFIGS = {
    {TowerType::INFANTRY_POST, {
        "Stanowisko Piechoty",
        200.0f,  // range
        15,      // damage
        4.0f,    // fireRate
        100,      // cost
        "assets/towers/Infantry_post1.png",
        150,     // upgradeCost
        {128.0f, 128.0f},  // size
        5,       // upgradeDamageDelta
        1.0f,    // upgradeFireRateDelta
        0.0f,    // explosionRadius
        1.0f     // edgeDamageMultiplier
    }},
    {TowerType::MACHINE_GUN, {
        "Karabin Maszynowy",
        230.0f,  // range
        10,      // damage
        6.0f,   // fireRate
        180,     // cost
        "assets/towers/Machine_gun1.png",
        220,     // upgradeCost
        {140.0f, 140.0f},  // size
        4,       // upgradeDamageDelta
        0.5f,    // upgradeFireRateDelta
        0.0f,    // explosionRadius
        1.0f     // edgeDamageMultiplier
    }},
    {TowerType::ARTILLERY, {
        "Artyleria",
        320.0f,  // range
        100,      // damage
        1.0f,    // fireRate
        300,     // cost
        "assets/towers/Artillery1.png",
        450,     // upgradeCost
        {164.0f, 164.0f},  // size
        15,      // upgradeDamageDelta
        0.2f,    // upgradeFireRateDelta
        90.0f,   // explosionRadius (px)
        0.35f    // edgeDamageMultiplier (35% dmg at the edge)
    }},
    {TowerType::AA_GUN, {
        "Dzialo Przeciwlotnicze",
        260.0f,  // range
        30,      // damage
        3.0f,    // fireRate
        250,     // cost
        "assets/towers/AA_gun1.png",
        320,     // upgradeCost
        {140.0f, 140.0f},  // size
        10,      // upgradeDamageDelta
        0.2f,    // upgradeFireRateDelta
        0.0f,    // explosionRadius
        1.0f     // edgeDamageMultiplier
    }}
};

// Konstruktor — nie ³aduje tekstury, ustawia pozycjê i zasiêg
Tower::Tower(float x, float y, TowerType type)
    : type(type),
      config(TOWER_CONFIGS.at(type)),
      level(1),
      showRange(false),
      texturePtr(nullptr),
      centerPos({x, y}) {

    sprite = nullptr;

    rangeCircle.setRadius(config.range);
    rangeCircle.setFillColor(sf::Color(255, 255, 255, 30));
    rangeCircle.setOutlineThickness(1.0f);
    rangeCircle.setOutlineColor(sf::Color(255, 255, 255, 100));
    rangeCircle.setPosition({ centerPos.x - config.range, centerPos.y - config.range });
}

void Tower::setTexture(const sf::Texture& tex) {
    texturePtr = &tex; // przechowujemy wskaŸnik do zewnêtrznej, trwa³ej tekstury
    sprite = std::make_unique<sf::Sprite>(*texturePtr);

    // Skalowanie do rozmiaru z konfiguracji
    sf::FloatRect bounds = sprite->getLocalBounds();
    if (bounds.size.x > 0 && bounds.size.y > 0) {
        float scaleX = config.size.x / bounds.size.x;
        float scaleY = config.size.y / bounds.size.y;
        sprite->setScale({ scaleX, scaleY });
    }

    // Ustaw origin na œrodek sprite, tak by rotacja by³a wokó³ œrodka
    sf::FloatRect local = sprite->getLocalBounds();
    sprite->setOrigin({ local.size.x / 2.0f, local.size.y / 2.0f });

    // Ustaw pozycjê sprite wzglêdem zarejestrowanego œrodka (centerPos)
    sprite->setPosition({ centerPos.x - config.size.x / 2.0f, centerPos.y - config.size.y / 2.0f });
    sprite->setPosition({ centerPos.x, centerPos.y });

    updateVisuals();
}

void Tower::rotateToDirection(const sf::Vector2f& dir) {
    if (!sprite) return;
    // dir is direction vector in world space; compute angle where 0deg = up (north)
    float angleRad = std::atan2(dir.y, dir.x); // angle where 0 = +x (east)
    float angleDeg = angleRad * 180.0f / 3.14159265f;
    // Our sprites point north (0 deg = up). atan2 gives 0 at east; convert:
    float rotation = angleDeg + 90.0f; // so that dir=(0,-1) -> rotation=0
    sprite->setRotation(sf::degrees(rotation));
}

void Tower::tickCooldown(float dt) {
    if (cooldownTimer > 0.0f) cooldownTimer -= dt;
}

bool Tower::canFire() const {
    return cooldownTimer <= 0.0f;
}

void Tower::resetCooldown() {
    cooldownTimer = 1.0f / config.fireRate;
}

// Rysowanie
void Tower::draw(sf::RenderWindow& window) const {
    if (showRange) {
        window.draw(rangeCircle);
    }
    if (sprite) {
        window.draw(*sprite);
    }
}

// Pozosta³e metody
void Tower::setPosition(float x, float y) {
    centerPos = { x, y };
    if (sprite) {
        sprite->setPosition({ x - config.size.x / 2.0f, y - config.size.y / 2.0f });
        sprite->setPosition({ x, y });
    }
    rangeCircle.setPosition({ x - config.range, y - config.range });
}

sf::Vector2f Tower::getPosition() const {
    return centerPos;
}

sf::FloatRect Tower::getBounds() const {
    if (sprite) return sprite->getGlobalBounds();
    // if no sprite, return rect centered at centerPos with size config.size
    return { { centerPos.x - config.size.x / 2.0f, centerPos.y - config.size.y / 2.0f }, { config.size.x, config.size.y } };
}

//Gettery
float Tower::getRange() const { return config.range; }
int Tower::getDamage() const { return config.damage; }
TowerType Tower::getType() const { return type; }
const std::string& Tower::getName() const { return config.name; }
int Tower::getCost() const { return config.cost; }
int Tower::getUpgradeCost() const { return config.upgradeCost; }

std::string Tower::getTexture() const { return config.textureFile; };

void Tower::toggleRangeDisplay(bool show) {
    showRange = show;
}

void Tower::upgrade() {
    if (!canUpgrade()) return;
    level++;
    config.damage += config.upgradeDamageDelta;
    config.fireRate += config.upgradeFireRateDelta;
    // Optionally adjust range slightly or keep constant
    // config.range *= 1.05f;
    updateVisuals();
}

void Tower::updateVisuals() {
    // Aktualizacja zasiêgu
    rangeCircle.setRadius(config.range);

    // Ustaw pozycjê zasiêgu wzglêdem centerPos
    rangeCircle.setPosition({ centerPos.x - config.range, centerPos.y - config.range });
}