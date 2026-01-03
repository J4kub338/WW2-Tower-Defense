#include "Game.hpp"
#include <iostream>
#include <cmath> 

static std::unique_ptr<Projectile> makeProjectileForTower(TowerType type, const sf::Vector2f& pos, const sf::Vector2f& dir, const std::unordered_map<TowerType, sf::Texture>& bulletTextures, int damage, float maxRange) {
    const sf::Texture* tex = nullptr;
    auto it = bulletTextures.find(type);
    if (it != bulletTextures.end()) tex = &it->second;
    float speed = 400.0f;
    switch (type) {
    case TowerType::INFANTRY_POST: speed = 700.0f; break;
    case TowerType::MACHINE_GUN: speed = 800.0f; break;
    case TowerType::ARTILLERY: speed = 500.0f; break;
    case TowerType::AA_GUN: speed = 900.0f; break;
    }
    return std::make_unique<Projectile>(pos, dir, speed, damage, tex, maxRange);
}

Game::Game()
    : window(sf::VideoMode({ 2560, 1600 }), "Tower Defense - WWII", State::Fullscreen),
      selectedTowerType(TowerType::INFANTRY_POST),
      showAllRanges(false),
      isRunning(true),
      HPText(font),
      CoinsText(font),
      lowCoinsText(font)
{

    // Inicjalizacja mapy
    if (!map.loadFromFile("maps/map1.txt")) {
        std::cout << "Tworzenie domyslnej mapy...\n";
    }

    // HUD texture
    const std::string hudPath = "assets/ui/hud.png";
    if (!hudTexture.loadFromFile(hudPath)) {
        std::cerr << "Nie mozna za³adowaæ HUD: " << hudPath << '\n';
        hudSprite.reset();
    } else {
        hudSprite = std::make_unique<sf::Sprite>(hudTexture);
        hudSprite->setPosition({ static_cast<float>(window.getSize().x) - static_cast<float>(hudTexture.getSize().x) - 10.0f, 10.0f });
    }

    // load bullet textures (best effort)
    bulletTextures[TowerType::INFANTRY_POST].loadFromFile("assets/bullets/IP_bullet.png");
    bulletTextures[TowerType::MACHINE_GUN].loadFromFile("assets/bullets/KM_bullet.png");
    bulletTextures[TowerType::ARTILLERY].loadFromFile("assets/bullets/ART_bullet.png");
    bulletTextures[TowerType::AA_GUN].loadFromFile("assets/bullets/AA_bullet.png");

    //HP text
    if (!font.openFromFile("assets/font/SFC_Rimowa.ttf")) {
        std::cerr << "Nie mo¿na za³adowaæ czcionki: assets/font/SFC_Rimowa.ttf" << std::endl;
        font = sf::Font(); // U¿yj domyœlnej czcionki SFML
    }
    if (!fontForText.openFromFile("assets/font/montserrat.semibold.ttf")) {
        std::cerr << "Nie mo¿na za³adowaæ czcionki: assets/font/SFC_Rimowa.ttf" << std::endl;
        font = sf::Font(); // U¿yj domyœlnej czcionki SFML
    }
    HPText.setString(to_string(PlayerHP) + "/100");
    HPText.setCharacterSize(40);
    HPText.setFillColor(sf::Color::White);
    HPText.setOutlineColor(sf::Color::Black);
    HPText.setOutlineThickness(4.0f);
    HPText.setOrigin({ HPText.getLocalBounds().size.x / 2.0f, HPText.getLocalBounds().size.y / 2.0f });
    HPText.setPosition({ 2325.0f, 28.0f });

    //Coins text
    CoinsText.setString(to_string(PlayerCoins));
    CoinsText.setCharacterSize(40);
    CoinsText.setFillColor(sf::Color::White);
    CoinsText.setOutlineColor(sf::Color::Black);
    CoinsText.setOutlineThickness(4.0f);
    CoinsText.setOrigin({ CoinsText.getLocalBounds().size.x / 2.0f, CoinsText.getLocalBounds().size.y / 2.0f });
    CoinsText.setPosition({ 1800.0f, 28.0f });

    // Low coins message setup (centered)
    lowCoinsText.setString("BRAKUJE MONET");
    lowCoinsText.setCharacterSize(60);
    lowCoinsText.setFillColor(sf::Color::Red);
    lowCoinsText.setOutlineColor(sf::Color::Black);
    lowCoinsText.setOutlineThickness(4.0f);
    // origin will be set later after local bounds are valid; we can center on screen when showing

    window.display();

    // Dodaj testowe wie¿e (u¿yj floatów, ¿eby unikn¹æ ostrze¿eñ konwersji)
    //towers.emplace_back(200.0f, 200.0f, TowerType::MACHINE_GUN);
    //towers.emplace_back(400.0f, 400.0f, TowerType::ARTILLERY);

    // W konstruktorze po za³adowaniu mapy - ³adujemy tekstury raz:
    for (const auto& item : TOWER_CONFIGS) {
        const TowerType type = item.first;
        const TowerConfig& cfg = item.second;
        sf::Texture tex;
        if (!tex.loadFromFile(cfg.textureFile)) {
            std::cerr << "Nie mozna zaladowac tekstury: " << cfg.textureFile << std::endl;
            continue;
        }
        towerTextures[type] = std::move(tex); // przechowujemy trwa³¹ kopiê
    }

    // Przygotuj fale (przyk³ad) - u¿ywamy SpawnEntry z per-entry interval i pathIndex
    Wave w1;
    w1.waveNumber = 1;
    w1.entries.push_back({ EnemyType::SOLDIER, 5, 0.5f, 0 });
    w1.entries.push_back({ EnemyType::LIGHT_PLANE, 5, 0.4f, 1 });
    w1.entries.push_back({ EnemyType::SOLDIER, 5, 0.5f, 0 });
    w1.entries.push_back({ EnemyType::MEDIUM_PLANE, 5, 0.6f, 1 });
    w1.entries.push_back({ EnemyType::HEAVY_PLANE, 5, 1.0f, 1 });
    w1.entries.push_back({ EnemyType::LIGHT_TANK, 2, 2.0f, 0 });
    w1.entries.push_back({ EnemyType::MEDIUM_TANK, 2, 2.5f, 0 });
    w1.entries.push_back({ EnemyType::HEAVY_TANK, 2, 4.0f, 0 });
    waveManager.addWave(w1);

    Wave w2;
    w2.waveNumber = 2;
    w2.entries.push_back({ EnemyType::SOLDIER, 10, 0.6f, 1 });
    w2.entries.push_back({ EnemyType::LIGHT_PLANE, 5, 0.5f, 0 });
    w2.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 0.5f, 1 });
    w2.entries.push_back({ EnemyType::HEAVY_PLANE, 3, 0.5f, 0 });
    w2.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 0.5f, 1 });
    w2.entries.push_back({ EnemyType::HEAVY_TANK, 6, 0.5f, 1 });
    w2.entries.push_back({ EnemyType::HEAVY_PLANE, 7, 0.5f, 0 });
    
    waveManager.addWave(w2);

    // UI: create simple colored rectangles for buttons
    startButton.setSize({ 160.0f, 40.0f });
    startButton.setFillColor(sf::Color(50, 200, 50));
    startButton.setPosition({ 20.0f, 20.0f });

    stopButton.setSize({ 160.0f, 40.0f });
    stopButton.setFillColor(sf::Color(200, 50, 50));
    stopButton.setPosition({ 200.0f, 20.0f });

    // Start first wave (disabled by default) -- don't auto-start
    // waveManager.startNextWave();
}

void Game::run() {
    sf::Clock clock;
    while (isRunning && window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
}

void Game::processEvents() {
    while (const auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            isRunning = false;
            window.close();
        }

        // Klikniêcie myszk¹
        if (auto mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseEvent->button == sf::Mouse::Button::Left) {
                // konwertuj piksele do koordynatów œwiata (wa¿ne przy view)
                sf::Vector2i pixelPos(mouseEvent->position.x, mouseEvent->position.y);
                sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);

                // Check buttons first (they are in window coordinates)
                sf::Vector2f mouseWindowPos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
                if (startButton.getGlobalBounds().contains(mouseWindowPos)) {
                    waveManager.startNextWave();
                    continue;
                }
                if (stopButton.getGlobalBounds().contains(mouseWindowPos)) {
                    waveManager.stopCurrentWave();
                    continue;
                }

                handleMouseClick(worldPos);
            }
        }

        // Klawiatura
        if (auto keyEvent = event->getIf<sf::Event::KeyPressed>()) {
            switch (keyEvent->code) {
            case sf::Keyboard::Key::Escape:
                isRunning = false;
                window.close();
                break;
            case sf::Keyboard::Key::Num1:
                selectedTowerType = TowerType::INFANTRY_POST;
                std::cout << "Wybrano: Karabin Maszynowy\n";
                break;
            case sf::Keyboard::Key::Num2:
                selectedTowerType = TowerType::MACHINE_GUN;
                std::cout << "Wybrano: Artyleria\n";
                break;
            case sf::Keyboard::Key::Num3:
                selectedTowerType = TowerType::ARTILLERY;
                std::cout << "Wybrano: Dzialo Przeciwlotnicze\n";
                break;
            case sf::Keyboard::Key::Num4:
                selectedTowerType = TowerType::AA_GUN;
                std::cout << "Wybrano: Stanowisko Piechoty\n";
                break;
            case sf::Keyboard::Key::R:
                showAllRanges = !showAllRanges;
                for (auto& tower : towers) {
                    tower.toggleRangeDisplay(showAllRanges);
                }
                break;
            case sf::Keyboard::Key::C:
                towers.clear();
                std::cout << "Wyczyszczono wieze\n";
                break;
            default:
                break;
            }
        }
    }
}

void Game::handleMouseClick(const sf::Vector2f& position) {
    sf::Vector2f center;

    // First check whether this click is on a buildable area (without claiming it)
    if (map.canBuildHere(position)) {
        int cost = TOWER_CONFIGS.at(selectedTowerType).cost;
        if (PlayerCoins < cost) {
            // not enough coins -> show message for a short time
            showLowCoinsMessage = true;
			soundManager.playNotEnoughCoinsSound();
            lowCoinsMessageTimer = 1.5f; // show for 1.5 seconds
            // center origin and position so text is centered on screen
            lowCoinsText.setOrigin({ lowCoinsText.getLocalBounds().size.x / 2.0f, lowCoinsText.getLocalBounds().size.y / 2.0f });
            lowCoinsText.setPosition({ window.getSize().x / 2.0f, window.getSize().y / 2.0f });
            std::cout << "Brakuje monet\n";
            return;
        }

        // We have enough coins; now try to claim area and build
        if (map.tryClaimBuildArea(position, center)) {
            // Deduct coins and update text
            PlayerCoins -= cost;
            CoinsText.setString(std::to_string(PlayerCoins));
            CoinsText.setOrigin({ CoinsText.getLocalBounds().size.x / 2.0f, CoinsText.getLocalBounds().size.y / 2.0f });

            towers.emplace_back(center.x + 32.0f, center.y + 32.0f, selectedTowerType);

            auto it = towerTextures.find(selectedTowerType);
            if (it != towerTextures.end()) {
                towers.back().setTexture(it->second); // referencja do trwa³ej tekstury
				soundManager.playTowerPlaceSound();
            } else {
                std::cerr << "Brak za³adowanej tekstury dla typu wie¿y\n";
            }

            towers.back().toggleRangeDisplay(showAllRanges);
            std::cout << "Postawiono wieze typu " << static_cast<int>(selectedTowerType)
                << " na (" << center.x << ", " << center.y << ")\n";
        } else {
            std::cout << "Nie mozna budowac w tym miejscu\n";
        }
    } else {
        std::cout << "Nie mozna budowac w tym miejscu\n";
    }
}

static void applyDamageToPlayer(EnemyType type, int& PlayerHP, sf::Text& HPText) {
    int damage = 0;
    switch (type) {
    case EnemyType::SOLDIER: damage = 5; break;
    case EnemyType::LIGHT_TANK: damage = 10; break;
    case EnemyType::MEDIUM_TANK: damage = 15; break;
    case EnemyType::HEAVY_TANK: damage = 20; break;
    case EnemyType::LIGHT_PLANE: damage = 8; break;
    case EnemyType::MEDIUM_PLANE: damage = 12; break;
    case EnemyType::HEAVY_PLANE: damage = 18; break;
    }
    PlayerHP -= damage;
    if (PlayerHP < 0) PlayerHP = 0;
    HPText.setString(std::to_string(PlayerHP) + "/100");
}

void Game::update(float deltaTime) {
    // Aktualizacja fal i generowanych przeciwników
    const auto& path1 = map.getPathPoints();
    const auto& path2 = map.getPathPoints2();

    waveManager.update(deltaTime, enemies, path1, path2);

    // Tower shooting
    for (auto& tower : towers) {
        tower.tickCooldown(deltaTime);
        // find nearest enemy in range
        const float range = tower.getRange();
        sf::Vector2f tpos = tower.getPosition();
        Enemy* target = nullptr;
        float bestDist = range * range;
        for (const auto& ePtr : enemies) {
            if (!ePtr) continue;
            sf::Vector2f ep = ePtr->getPosition();
            float dx = ep.x - tpos.x;
            float dy = ep.y - tpos.y;
            float d2 = dx*dx + dy*dy;
            if (d2 <= bestDist) {
                bestDist = d2;
                target = ePtr.get();
            }
        }
        if (target && tower.canFire()) {
            sf::Vector2f dir = target->getPosition() - tpos;
            tower.rotateToDirection(dir);
            auto proj = makeProjectileForTower(tower.getType(), tpos, dir, bulletTextures, tower.getDamage(), range);
            projectiles.push_back(std::move(proj));
            tower.resetCooldown();
        } else if (target) {
            // still rotate to target even if cooling down
            sf::Vector2f dir = target->getPosition() - tpos;
            tower.rotateToDirection(dir);
        }
    }

    // Update projectiles
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        auto& p = *it;
        p->update(deltaTime);
        bool removed = false;
        // remove projectile if it exceeded its max range
        if (!p->isAlive()) {
            it = projectiles.erase(it);
            continue;
        }
        // check collision with enemies (simple distance check)
        for (auto& ePtr : enemies) {
            if (!ePtr) continue;
            float dx = ePtr->getPosition().x - p->getPosition().x;
            float dy = ePtr->getPosition().y - p->getPosition().y;
            float dist2 = dx*dx + dy*dy;
            if (dist2 < 20.0f * 20.0f) {
                ePtr->takeDamage(p->getDamage());
                // mark projectile dead by erasing
                it = projectiles.erase(it);
                removed = true;
                break;
            }
        }
        if (!removed) ++it;
    }

    // Aktualizacja przeciwników
    for (auto it = enemies.begin(); it != enemies.end();) {
        auto& e = *it;
        e->update(deltaTime);

        if (e->isDead()) {
            // If enemy is dead but still has positive health -> it reached the end (escaped)
            if (e->getHealth() > 0) {
                applyDamageToPlayer(e->getType(), PlayerHP, HPText);
            } else {
                // enemy died normally -> give reward
                PlayerCoins += e->getReward();
                CoinsText.setString(std::to_string(PlayerCoins));
                CoinsText.setOrigin({ CoinsText.getLocalBounds().size.x / 2.0f, CoinsText.getLocalBounds().size.y / 2.0f });
            }
            // Otherwise enemy died normally (could give reward here)
            it = enemies.erase(it);
        } else {
            ++it;
        }
    }

    // update low coins message timer
    if (showLowCoinsMessage) {
        lowCoinsMessageTimer -= deltaTime;
        if (lowCoinsMessageTimer <= 0.0f) {
            showLowCoinsMessage = false;
        }
    }
}

void Game::HPTextUpdate() {

    // Safer check for enemies near path end (avoid exact equality and duplicated decrements)
    sf::Vector2f end = map.getPathEnd();
    const float radius = 50.0f;

    for (const auto& e : enemies) {
        sf::Vector2f pos = e->getPosition();
        float dx = pos.x - end.x;
        float dy = pos.y - end.y;
        if (dx*dx + dy*dy <= radius*radius) {
            // Treat as escaped
            // Reduce HP and update text, then break (to avoid multiple decrements in one call)
            applyDamageToPlayer(e->getType(), PlayerHP, HPText);
            break;
        }
    }

}

void Game::render() {
    
    Sprite backgroundSprite(map.getBackgroundTexture());
    window.clear(sf::Color::Black);
    window.draw(backgroundSprite);

    // 1. T³o (Map::draw rysuje t³o jeœli istnieje)
    map.draw(window);

    // 3. Wie¿e
    for (const auto& tower : towers) {
        tower.draw(window);
    }

    // 4. Przeciwnicy
    for (const auto& e : enemies) {
        e->draw(window);
    }
    for (const auto& p : projectiles) p->draw(window);

    // Draw UI (on top) and creating Sprite for HUD
    if (hudSprite) { // Check if hudSprite is valid
        Sprite hudSprite(hudTexture);
        hudSprite.setPosition({ (float)window.getSize().x - (float)hudTexture.getSize().x, 0.0f});
        hudSprite.setTexture(hudTexture);
        window.draw(hudSprite);
    }
    window.draw(startButton);
    window.draw(stopButton);
    window.draw(HPText);
    window.draw(CoinsText);

    // draw low coins text if active
    if (showLowCoinsMessage) {
        window.draw(lowCoinsText);
    }

    window.display();
}


