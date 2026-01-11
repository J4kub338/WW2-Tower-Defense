#include "Menu.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>

namespace {
    constexpr float kScreenW = 2560.0f;
    constexpr float kScreenH = 1600.0f;

    static void fitToScreen(sf::Sprite& sprite, const sf::Texture& texture) {
        const auto sz = texture.getSize();
        if (sz.x == 0 || sz.y == 0) return;

        sprite.setPosition({ 0.f, 0.f });
        sprite.setScale({ kScreenW / static_cast<float>(sz.x), kScreenH / static_cast<float>(sz.y) });
    }

    static void drawFallbackBackground(sf::RenderWindow& window, sf::Color color = sf::Color(30, 30, 30)) {
        sf::RectangleShape rect;
        rect.setSize({ static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y) });
        rect.setFillColor(color);
        window.draw(rect);
    }
} // namespace

Menu::Menu()
    : btnPlay(nullptr),
      btnSettings(nullptr),
      btnQuit(nullptr),
      btnBack(nullptr)
{
    currentState = MenuState::MAIN_MENU;
    settings.volume = 50.0f;
    settings.fullscreen = true; // Zawsze fullscreen dla 2560x1600

    // --- T£A ---
    const bool mainLoaded = bgTextureMain.loadFromFile("assets/ui/menu_bg.png");
    if (!mainLoaded) {
        std::cerr << "[MENU] Brak t³a: assets/ui/menu_bg.png (sprawdŸ Working Directory / folder assets).\n";
    }

    bool mapsLoaded = bgTextureMaps.loadFromFile("assets/ui/menu_maps_bg.png");
    if (!mapsLoaded) {
        std::cerr << "[MENU] Brak t³a: assets/ui/menu_maps_bg.png (u¿ywam t³a g³ównego jeœli dostêpne).\n";
        if (mainLoaded) {
            bgTextureMaps = bgTextureMain; // fallback
            mapsLoaded = true;
        }
    }

    // IMPORTANT (SFML 3): sf::Sprite has no default ctor; create only after texture is valid.
    if (mainLoaded) {
        bgSpriteMain = std::make_unique<sf::Sprite>(bgTextureMain);
        fitToScreen(*bgSpriteMain, bgTextureMain);
        const auto sz = bgTextureMain.getSize();
        std::cerr << "[MENU] Za³adowano menu_bg.png: " << sz.x << "x" << sz.y << "\n";
    } else {
        bgSpriteMain.reset();
    }

    if (mapsLoaded) {
        bgSpriteMaps = std::make_unique<sf::Sprite>(bgTextureMaps);
        fitToScreen(*bgSpriteMaps, bgTextureMaps);
    } else {
        bgSpriteMaps.reset();
    }

    // --- PRZYCISKI ---
    bool buttonsLoaded = true;
    if (!playIdleTex.loadFromFile("assets/ui/btn_play_idle.png")) buttonsLoaded = false;
    if (!playPressedTex.loadFromFile("assets/ui/btn_play_pressed.png")) buttonsLoaded = false;
    if (!settingsIdleTex.loadFromFile("assets/ui/btn_settings_idle.png")) buttonsLoaded = false;
    if (!settingsPressedTex.loadFromFile("assets/ui/btn_settings_pressed.png")) buttonsLoaded = false;

    // NEW: Quit button (idle/pressed). If you only have one texture, both can point to it.
    bool quitLoaded = true;
    if (!quitIdleTex.loadFromFile("assets/ui/btn_quit.png")) quitLoaded = false;
    if (!quitPressedTex.loadFromFile("assets/ui/btn_quit.png")) quitLoaded = false;

    // NEW: Back button for level select
    const bool backLoaded = backTex.loadFromFile("assets/ui/btn_back.png");
    if (!backLoaded) {
        std::cerr << "[MENU] Nie za³adowano przycisku powrotu: assets/ui/btn_back.png\n";
    }

    if (!buttonsLoaded) {
        std::cerr << "[MENU] Nie za³adowano wszystkich tekstur przycisków (menu dalej dzia³a, ale mo¿e byæ puste).\n";
    }
    if (!quitLoaded) {
        std::cerr << "[MENU] Nie za³adowano przycisku wyjœcia: assets/ui/btn_quit.png\n";
    }

    // --- MINIATURY MAP ---
    mapThumbTextures.resize(6);
    for (int i = 0; i < 6; ++i) {
        const std::string filename = "assets/ui/map_thumb_" + std::to_string(i + 1) + ".png";
        if (!mapThumbTextures[i].loadFromFile(filename)) {
            std::cerr << "[MENU] Brak miniatury: " << filename << "\n";
        }
    }

    initMainButtons();
    initMapButtons();
}

Menu::~Menu() {
    delete btnPlay;
    delete btnSettings;
    delete btnQuit;
    for (auto* btn : mapButtons) delete btn;
    delete btnBack;
}

void Menu::initMainButtons() {
    // Przycisk ma origin w œrodku, wiêc podajemy œrodek.
    const float centerX = kScreenW / 2.0f;

    const float startY = 800.0f;
    const float gap = 250.0f;

    // W razie ponownej inicjalizacji
    delete btnPlay;
    btnPlay = nullptr;
    delete btnSettings;
    btnSettings = nullptr;
    delete btnQuit;
    btnQuit = nullptr;

    if (playIdleTex.getSize().x > 0 && playPressedTex.getSize().x > 0) {
        btnPlay = new Button(centerX, startY, &playIdleTex, &playPressedTex);
    }

    if (settingsIdleTex.getSize().x > 0 && settingsPressedTex.getSize().x > 0) {
        btnSettings = new Button(centerX, startY + gap, &settingsIdleTex, &settingsPressedTex);
    }

    // NEW: Quit button under settings
    if (quitIdleTex.getSize().x > 0 && quitPressedTex.getSize().x > 0) {
        btnQuit = new Button(centerX, startY + 2.0f * gap, &quitIdleTex, &quitPressedTex);
    }
}

void Menu::initMapButtons() {
    for (auto* btn : mapButtons) delete btn;
    mapButtons.clear();

    delete btnBack;
    btnBack = nullptr;

    // Konfiguracja dla miniatur 600x400 na ekranie 2560x1600
    const float thumbW = 600.0f;
    const float thumbH = 400.0f;
    const float gapX = 150.0f;
    const float gapY = 100.0f;

    // Wyliczone pod 2560x1600, z origin w œrodku.
    const float startCenterX = 530.0f;
    const float startCenterY = 750.0f;

    for (int i = 0; i < 6; ++i) {
        const int row = i / 3;
        const int col = i % 3;

        const float x = startCenterX + (col * (thumbW + gapX));
        const float y = startCenterY + (row * (thumbH + gapY));

        if (i < static_cast<int>(mapThumbTextures.size()) && mapThumbTextures[i].getSize().x > 0) {
            Button* btn = new Button(x, y, &mapThumbTextures[i], &mapThumbTextures[i]);
            mapButtons.push_back(btn);
        }
    }

    // NEW: Back button in level select (bottom-left)
    if (backTex.getSize().x > 0) {
        const float margin = 80.0f;
        const float bx = margin + backTex.getSize().x / 2.0f;
        const float by = kScreenH - margin - backTex.getSize().y / 2.0f;
        btnBack = new Button(bx - 10.0f, by + 45.0f, &backTex, &backTex);
    }
}

int Menu::update(const sf::Vector2f& mousePos, bool isMousePressed) {
    if (currentState == MenuState::MAIN_MENU) {
        if (btnPlay) btnPlay->update();
        if (btnSettings) btnSettings->update();
        if (btnQuit) btnQuit->update();

        if (btnPlay && btnPlay->isPressed(mousePos, isMousePressed)) {
            return 1; // Prze³¹cz na wybór mapy
        }
        if (btnSettings && btnSettings->isPressed(mousePos, isMousePressed)) {
            return 0; // Atrapa
        }
        if (btnQuit && btnQuit->isPressed(mousePos, isMousePressed)) {
            return 2; // quit
        }
    }
    else if (currentState == MenuState::LEVEL_SELECT) {
        for (int i = 0; i < static_cast<int>(mapButtons.size()); ++i) {
            mapButtons[i]->update();
            if (mapButtons[i]->isPressed(mousePos, isMousePressed)) {
                return 100 + i; // Wybór mapy (100 = Mapa 1, 101 = Mapa 2...)
            }
        }

        if (btnBack) {
            btnBack->update();
            if (btnBack->isPressed(mousePos, isMousePressed)) {
                return 3; // back to main menu
            }
        }
    }

    return 0;
}

void Menu::switchToLevelSelect() {
    currentState = MenuState::LEVEL_SELECT;
}

void Menu::switchToMainMenu() {
    currentState = MenuState::MAIN_MENU;
}

void Menu::draw(sf::RenderWindow& window) {
    if (currentState == MenuState::MAIN_MENU) {
        if (bgSpriteMain) window.draw(*bgSpriteMain);
        else drawFallbackBackground(window);

        if (btnPlay) btnPlay->draw(window);
        if (btnSettings) btnSettings->draw(window);
        if (btnQuit) btnQuit->draw(window);
    }
    else if (currentState == MenuState::LEVEL_SELECT) {
        if (bgSpriteMaps) window.draw(*bgSpriteMaps);
        else drawFallbackBackground(window);

        for (auto* btn : mapButtons) {
            btn->draw(window);
        }
        if (btnBack) btnBack->draw(window);
    }
}