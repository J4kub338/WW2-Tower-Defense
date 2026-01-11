#include "ExplosionEffect.hpp"
#include <algorithm>

ExplosionEffect::ExplosionEffect(const sf::Vector2f& c, float r,
                                 const std::vector<const sf::Texture*>& f,
                                 float frameTimeSeconds)
    : center(c), radius(r), frames(f), frameTimeSeconds(frameTimeSeconds) {

    if (frames.empty() || frames[0] == nullptr) {
        alive = false;
        return;
    }

    sprite = std::make_unique<sf::Sprite>(*frames[0]);
    applyFrame();
}

void ExplosionEffect::applyScaleToRadius() {
    if (!sprite) return;

    const sf::FloatRect lb = sprite->getLocalBounds();
    if (lb.size.x <= 0.0f || lb.size.y <= 0.0f) return;

    const float desiredDiameter = std::max(1.0f, radius * 2.0f);
    const float scaleX = desiredDiameter / lb.size.x;
    const float scaleY = desiredDiameter / lb.size.y;
    sprite->setScale({ scaleX, scaleY });
}

void ExplosionEffect::applyFrame() {
    if (!sprite) {
        alive = false;
        return;
    }

    if (frames.empty() || frameIndex >= frames.size() || frames[frameIndex] == nullptr) {
        alive = false;
        return;
    }

    sprite->setTexture(*frames[frameIndex], true);

    const sf::FloatRect lb = sprite->getLocalBounds();
    sprite->setOrigin({ lb.size.x / 2.0f, lb.size.y / 2.0f });
    sprite->setPosition(center);

    applyScaleToRadius();

    // optional: make it a bit transparent so it acts as "range indicator"
    sprite->setColor(sf::Color(255, 255, 255, 200));
}

void ExplosionEffect::update(float dt) {
    if (!alive) return;

    elapsed += dt;
    while (elapsed >= frameTimeSeconds) {
        elapsed -= frameTimeSeconds;
        ++frameIndex;
        if (frameIndex >= frames.size()) {
            alive = false;
            return;
        }
        applyFrame();
    }
}

void ExplosionEffect::draw(sf::RenderWindow& window) const {
    if (!alive) return;
    if (sprite) window.draw(*sprite);
}
