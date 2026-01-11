#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class ExplosionEffect {
public:
    ExplosionEffect(const sf::Vector2f& center, float radius,
                    const std::vector<const sf::Texture*>& frames,
                    float frameTimeSeconds = 0.06f);

    void update(float dt);
    void draw(sf::RenderWindow& window) const;

    bool isAlive() const { return alive; }

private:
    sf::Vector2f center;
    float radius;

    std::vector<const sf::Texture*> frames;
    float frameTimeSeconds;

    float elapsed = 0.0f;
    std::size_t frameIndex = 0;
    bool alive = true;

    std::unique_ptr<sf::Sprite> sprite; // SFML 3: no default ctor

    void applyFrame();
    void applyScaleToRadius();
};
