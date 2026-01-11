#pragma once
#include <SFML/Graphics.hpp>
#include "Button.hpp"
#include <vector>
#include <memory>

// 1. Struktura ustawieñ
struct GameSettings {
    float volume;
    bool fullscreen;
};

// 2. Enum stanów (G³ówne menu / Wybór mapy)
enum class MenuState {
    MAIN_MENU,
    LEVEL_SELECT
};

class Menu {
private:
    // 3. Zmienna stanu (To naprawi b³êdy "currentState undeclared")
    MenuState currentState;

    // T³a
    sf::Texture bgTextureMain;
    std::unique_ptr<sf::Sprite> bgSpriteMain;

    sf::Texture bgTextureMaps;
    std::unique_ptr<sf::Sprite> bgSpriteMaps;

    // Tekstury przycisków g³ównego menu
    sf::Texture playIdleTex;
    sf::Texture playPressedTex;
    sf::Texture settingsIdleTex;
    sf::Texture settingsPressedTex;

    // NEW: Quit button textures
    sf::Texture quitIdleTex;
    sf::Texture quitPressedTex;

    // NEW: Back button texture (same idle/pressed)
    sf::Texture backTex;

    // Przyciski g³ównego menu
    Button* btnPlay;
    Button* btnSettings;
    Button* btnQuit;

    // Zasoby wyboru mapy
    std::vector<sf::Texture> mapThumbTextures;
    std::vector<Button*> mapButtons;
    Button* btnBack;

    // 4. Ustawienia gry (Musi byæ TYLKO RAZ - to naprawi b³¹d "redefinition")
    GameSettings settings;

    void initMainButtons();
    void initMapButtons();

public:
    Menu();
    ~Menu();

    // Funkcja do zmiany stanu (wywo³ywana w Game.cpp)
    void switchToLevelSelect();
    void switchToMainMenu();

    // Return codes:
    // 0 = no action
    // 1 = go to level select
    // 2 = quit game
    // 100+i = start map i
    // 3 = back to main menu
    int update(const sf::Vector2f& mousePos, bool isMousePressed);

    // Rysowanie
    void draw(sf::RenderWindow& window);

    GameSettings getSettings() const { return settings; }
};