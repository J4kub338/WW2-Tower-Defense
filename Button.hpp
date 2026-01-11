#pragma once
#include <SFML/Graphics.hpp>

class Button {
private:
    sf::Sprite sprite;

    // WskaŸniki do tekstur
    const sf::Texture* idleTexture;
    const sf::Texture* pressedTexture;

    bool isPressedState = false;

public:
    // Konstruktor przyjmuje teraz TYLKO pozycjê i dwie tekstury
    Button(float x, float y, const sf::Texture* idleTex, const sf::Texture* pressedTex);

    void update();
    void draw(sf::RenderWindow& window);

    // Sprawdza klikniêcie
    bool isPressed(const sf::Vector2f& mousePos, bool isMouseClicked);

    // Helper do pobrania rozmiaru (¿eby ³adnie uk³adaæ przyciski)
    sf::Vector2f getSize() const;
};