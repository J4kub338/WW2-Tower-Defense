#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

class Projectile {
public:
    // pos = start position, dir = direction vector, speed, damage, tex optional, maxRange in pixels
    Projectile(const sf::Vector2f& pos, const sf::Vector2f& dir, float speed, int damage, const sf::Texture* tex, float maxRange = 1000.0f,
               float explosionRadius = 0.0f, float edgeDamageMultiplier = 1.0f);

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    bool isAlive() const { return alive; }
    sf::Vector2f getPosition() const { return position; }
    int getDamage() const { return damage; }

    // AoE info
    bool hasExplosion() const { return explosionRadius > 0.0f; }
    float getExplosionRadius() const { return explosionRadius; }
    float getEdgeDamageMultiplier() const { return edgeDamageMultiplier; }

private:
    sf::Vector2f position;
    sf::Vector2f origin;
    sf::Vector2f velocity;
    float speed;
    int damage;
    bool alive = true;
    float maxRange = 1000.0f;
    float elapsedTime = 0.0f; // seconds since spawn
    float maxLifetime = 5.0f; // seconds, computed from maxRange/speed

    // AoE
    float explosionRadius = 0.0f;
    float edgeDamageMultiplier = 1.0f;

    std::unique_ptr<sf::Sprite> sprite;
};
