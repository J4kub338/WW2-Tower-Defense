#include "Projectile.hpp"
#include <cmath>

Projectile::Projectile(const sf::Vector2f& pos, const sf::Vector2f& dir, float spd, int dmg, const sf::Texture* tex, float maxRange)
    : position(pos), origin(pos), speed(spd), damage(dmg), maxRange(maxRange) {
    sf::Vector2f n = dir;
    float len = std::sqrt(n.x*n.x + n.y*n.y);
    if (len > 0.0f) n /= len;
    velocity = n * speed;
    if (tex) {
        sprite = std::make_unique<sf::Sprite>(*tex);
        // set origin to center for rotation
        sf::FloatRect lb = sprite->getLocalBounds();
        sprite->setOrigin({ lb.size.x/2.0f, lb.size.y/2.0f });
        sprite->setPosition(position);
        float angle = std::atan2(n.y, n.x) * 180.0f / 3.14159265f;
        sprite->setRotation(sf::degrees(angle + 90.0f)); // assuming texture points up
    }
    // compute lifetime from range/speed (plus small epsilon)
    if (speed > 0.0f) {
        maxLifetime = maxRange / speed + 0.1f;
    } else {
        maxLifetime = 5.0f;
    }
}

void Projectile::update(float dt) {
    if (!alive) return;
    elapsedTime += dt;
    position += velocity * dt;
    if (sprite) sprite->setPosition(position);

    // remove if too far from origin
    float dx = position.x - origin.x;
    float dy = position.y - origin.y;
    if (dx*dx + dy*dy > maxRange*maxRange) {
        alive = false;
        return;
    }
    // or lifetime exceeded
    if (elapsedTime > maxLifetime) {
        alive = false;
    }
}

void Projectile::draw(sf::RenderWindow& window) const {
    if (!alive) return;
    if (sprite) window.draw(*sprite);
}
