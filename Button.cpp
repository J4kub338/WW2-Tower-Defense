#include "Button.hpp"

Button::Button(float x, float y, const sf::Texture* idleTex, const sf::Texture* pressedTex)
    : sprite(*idleTex) // SFML 3.0: Inicjalizacja sprite tekstur¹
{
    this->idleTexture = idleTex;
    this->pressedTexture = pressedTex;

    // Ustawiamy origin na œrodek przycisku, ¿eby ³atwiej by³o centrowaæ
    sf::FloatRect bounds = sprite.getLocalBounds();
    sprite.setOrigin({ bounds.size.x / 2.0f, bounds.size.y / 2.0f });

    sprite.setPosition({ x, y });
}

sf::Vector2f Button::getSize() const {
    sf::FloatRect bounds = sprite.getGlobalBounds();
    return { bounds.size.x, bounds.size.y };
}

void Button::update() {
    // Reset do zwyk³ego wygl¹du w nowej klatce
    if (!isPressedState) {
        sprite.setTexture(*idleTexture);
    }
    isPressedState = false;
}

void Button::draw(sf::RenderWindow& window) {
    window.draw(sprite);
}

bool Button::isPressed(const sf::Vector2f& mousePos, bool isMouseClicked) {
    if (sprite.getGlobalBounds().contains(mousePos)) {
        if (isMouseClicked) {
            sprite.setTexture(*pressedTexture); // Zmiana grafiki na wciœniêt¹
            isPressedState = true;
            return true;
        }
    }
    return false;
}