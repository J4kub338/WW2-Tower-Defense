#include "Game.hpp"
#include <iostream>
#include <cmath> 

static std::unique_ptr<Projectile> makeProjectileForTower(TowerType type, const sf::Vector2f& pos, const sf::Vector2f& dir, const std::unordered_map<TowerType, sf::Texture>& bulletTextures, int damage, float maxRange) {
    const sf::Texture* tex = nullptr;
    auto it = bulletTextures.find(type);
    if (it != bulletTextures.end()) tex = &it->second;
    float speed = 400.0f;
    switch (type) {
    case TowerType::INFANTRY_POST: speed = 900.0f; break;
    case TowerType::MACHINE_GUN: speed = 1000.0f; break;
    case TowerType::ARTILLERY: speed = 900.0f; break;
    case TowerType::AA_GUN: speed = 900.0f; break;
    }
    return std::make_unique<Projectile>(pos, dir, speed, damage, tex, maxRange);
}

void Game::loadMapByIndex(int index) {
    if (index < 0 || index >= static_cast<int>(availableMaps.size())) return;
    selectedMapIndex = index;
    const MapInfo& info = availableMaps[index];

    // load map file
    if (!map.loadFromFile(info.mapFile)) {
        std::cout << "Tworzenie domyslnej mapy...\n";
    }

    // reset wave manager and copy waves from map info
    waveManager = WaveManager();
    for (const auto& w : info.waves) {
        waveManager.addWave(w);
    }

    // Start or switch background music for selected map
    soundManager.playBackground("music/track" + std::to_string(info.Soundtracknumber) + ".mp3", true, 60.0f);

    // store current map prefix for spawn-time use
    currentMapEnemyPrefix = info.enemyTexturePrefix;

    // apply map-specific starting coins
    PlayerCoins = info.playerCoins;
    CoinsText.setString(std::to_string(PlayerCoins));
    CoinsText.setOrigin({ CoinsText.getLocalBounds().size.x / 2.0f, CoinsText.getLocalBounds().size.y / 2.0f });

    KillsCount = 0;
	KillsText.setString(to_string(KillsCount));
	// Reset current wave number
	currentWaveNumber = 0;
	numberOfWaves = static_cast<int>(info.waves.size());
	WaveText.setString(to_string(currentWaveNumber) + "/" + to_string(info.waves.size()));
	

    gameOver = false;
    victory = false;

    // For now we only store enemy texture prefix in map info and pass it when spawning enemies via WaveManager.
    // WaveManager currently creates enemies without custom texture names. A full implementation would propagate texture overrides when creating Enemy instances.
}
Game::Game()
// Ustawiamy okno na sztywno 2560x1600 w trybie okienkowym (Default)
    : window(sf::VideoMode({ 2560, 1600 }), "Tower Defense - WWII", sf::Style::Default, sf::State::Windowed),
    selectedTowerType(TowerType::INFANTRY_POST),
    showAllRanges(false),
    isRunning(true),
    HPText(font),
    CoinsText(font),
    lowCoinsText(font),
    WaveText(font),
    KillsText(font),
    MaxLevelText(font)
{
    menu = std::make_unique<Menu>();
    isMenuState = true; // Zaczynamy w menu
    soundManager.setBackgroundVolume(50.0f);
    // Setup available maps (basic example)
    //Kampania wrzeœniowa
    MapInfo m1;
	m1.playerCoins = 600;
    m1.mapFile = "maps/map1.txt";
    m1.enemyTexturePrefix = "GER";
	m1.Soundtracknumber = 3;
    // example waves - these would normally be loaded from a per-map data file
    Wave waveA1;
    waveA1.waveNumber = 1; 
    waveA1.entries.push_back({ EnemyType::SOLDIER, 3, 0.5f, 0 });
    waveA1.entries.push_back({ EnemyType::SOLDIER, 3, 0.5f, 1 });
    waveA1.entries.push_back({ EnemyType::SOLDIER, 3, 0.5f, 0 });
    waveA1.entries.push_back({ EnemyType::SOLDIER, 3, 0.5f, 0 });
    waveA1.entries.push_back({ EnemyType::SOLDIER, 3, 0.5f, 1 });
    waveA1.entries.push_back({ EnemyType::SOLDIER, 3, 0.5f, 0 });
    m1.waves.push_back(waveA1);
	Wave waveA2;
	waveA2.waveNumber = 2;
    waveA2.entries.push_back({ EnemyType::SOLDIER, 3, 0.5f, 0 });
    waveA2.entries.push_back({ EnemyType::SOLDIER, 3, 0.5f, 1 });
    waveA2.entries.push_back({ EnemyType::LIGHT_TANK, 2, 1.0f, 1 });
    waveA2.entries.push_back({ EnemyType::LIGHT_TANK, 2, 1.0f, 0 });
    waveA2.entries.push_back({ EnemyType::SOLDIER, 5, 0.8f, 0 });
    waveA2.entries.push_back({ EnemyType::SOLDIER, 5, 0.8f, 1 });
	m1.waves.push_back(waveA2);
    Wave waveA3;
	waveA3.waveNumber = 3;
	waveA3.entries.push_back({ EnemyType::SOLDIER, 5, 0.7f, 0 });
    waveA3.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 1 });
    waveA3.entries.push_back({ EnemyType::LIGHT_PLANE, 2, 2.0f, 0 });
    waveA3.entries.push_back({ EnemyType::SOLDIER, 5, 0.4f, 1 });
    waveA3.entries.push_back({ EnemyType::LIGHT_TANK, 3, 1.5f, 0 });
	waveA3.entries.push_back({ EnemyType::LIGHT_PLANE, 2, 2.0f, 1 });
	waveA3.entries.push_back({ EnemyType::SOLDIER, 5, 0.7f, 1 });
	m1.waves.push_back(waveA3);
	Wave waveA4;
	waveA4.waveNumber = 4;
    waveA4.entries.push_back({ EnemyType::LIGHT_TANK, 4, 2.0f, 0 });
    waveA4.entries.push_back({ EnemyType::LIGHT_TANK, 6, 3.0f, 1 });
    waveA4.entries.push_back({ EnemyType::MEDIUM_PLANE, 4, 2.5f, 0 });
    waveA4.entries.push_back({ EnemyType::MEDIUM_PLANE, 2, 3.0f, 1 });
	waveA4.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 0 });
	waveA4.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 1 });
	waveA4.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 0 });
	waveA4.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 1 });
	waveA4.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 0 });
	waveA4.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 1 });
	waveA4.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 0 });
	waveA4.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 1 });
	waveA4.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 0 });
	waveA4.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.5f, 1 });
	m1.waves.push_back(waveA4);
	availableMaps.push_back(m1);

    //Bitwa w Afryce
    MapInfo m2;
    m2.mapFile = "maps/map2.txt";
    m2.enemyTexturePrefix = "UK";
    m2.playerCoins = 850;
    m2.Soundtracknumber = 2;
    Wave waveB1;
    waveB1.waveNumber = 1; 
    waveB1.entries.push_back({ EnemyType::SOLDIER, 5, 0.9f, 0 });
    waveB1.entries.push_back({ EnemyType::LIGHT_TANK, 3, 2.0f, 1 });
    waveB1.entries.push_back({ EnemyType::LIGHT_TANK, 3, 2.3f, 0 });
    waveB1.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 3.0f, 1 });
    waveB1.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 2.0f, 0 });
    waveB1.entries.push_back({ EnemyType::SOLDIER, 3, 2.6f, 1 });
    waveB1.entries.push_back({ EnemyType::SOLDIER, 3, 3.0f, 0 });
    m2.waves.push_back(waveB1);
	Wave waveB2;
	waveB2.waveNumber = 2;
	waveB2.entries.push_back({ EnemyType::SOLDIER, 10, 0.4f, 0 });
	waveB2.entries.push_back({ EnemyType::SOLDIER, 10, 0.4f, 1 });
	waveB2.entries.push_back({ EnemyType::LIGHT_TANK, 4, 2.5f, 0 });
	waveB2.entries.push_back({ EnemyType::LIGHT_TANK, 4, 2.5f, 1 });    
	waveB2.entries.push_back({ EnemyType::LIGHT_PLANE, 4, 1.5f, 0 });
	waveB2.entries.push_back({ EnemyType::LIGHT_PLANE, 4, 1.5f, 1 });
	waveB2.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 2.9f, 0 });
	waveB2.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 2.9f, 1 });
	waveB1.entries.push_back({ EnemyType::SOLDIER, 3, 0.4f, 0 });
	waveB2.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 2.0f, 1 });
	waveB2.entries.push_back({ EnemyType::LIGHT_PLANE, 2, 1.5f, 0 });
	waveB2.entries.push_back({ EnemyType::SOLDIER, 3, 0.4f, 1 });
	waveB2.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 2.0f, 0 });
	m2.waves.push_back(waveB2);
	Wave waveB3;
	waveB3.waveNumber = 3;
	waveB3.entries.push_back({ EnemyType::SOLDIER, 3, 0.3f, 0 });
	waveB3.entries.push_back({ EnemyType::SOLDIER, 3, 0.3f, 1 });
	waveB3.entries.push_back({ EnemyType::MEDIUM_TANK, 4, 2.5f, 0 });
	waveB3.entries.push_back({ EnemyType::LIGHT_TANK, 4, 2.5f, 1 });
	waveB3.entries.push_back({ EnemyType::MEDIUM_TANK, 2, 3.5f, 0 });
	waveB3.entries.push_back({ EnemyType::HEAVY_TANK, 2, 3.5f, 1 });
	waveB3.entries.push_back({ EnemyType::MEDIUM_PLANE, 4, 1.5f, 0 });
	waveB3.entries.push_back({ EnemyType::LIGHT_PLANE, 4, 1.5f, 1 });
	waveB3.entries.push_back({ EnemyType::SOLDIER, 10, 2.9f, 0 });
	waveB3.entries.push_back({ EnemyType::HEAVY_TANK, 3, 3.9f, 1 });
	waveB3.entries.push_back({ EnemyType::HEAVY_TANK, 3, 3.9f, 0 });
	m2.waves.push_back(waveB3);
	Wave waveB4;
	waveB4.waveNumber = 4;
	waveB4.entries.push_back({ EnemyType::SOLDIER, 2, 0.2f, 0 });
	waveB4.entries.push_back({ EnemyType::SOLDIER, 2, 0.2f, 1 });
	waveB4.entries.push_back({ EnemyType::SOLDIER, 2, 0.2f, 0 });
	waveB4.entries.push_back({ EnemyType::SOLDIER, 2, 0.2f, 1 });
	waveB4.entries.push_back({ EnemyType::LIGHT_PLANE, 4, 0.5f, 0 });
	waveB4.entries.push_back({ EnemyType::SOLDIER, 2, 0.2f, 0 });
	waveB4.entries.push_back({ EnemyType::SOLDIER, 2, 0.2f, 1 });
	waveB4.entries.push_back({ EnemyType::SOLDIER, 2, 0.2f, 0 });
	waveB4.entries.push_back({ EnemyType::MEDIUM_TANK, 4, 3.0f, 1 });
	waveB4.entries.push_back({ EnemyType::HEAVY_PLANE, 3, 3.0f, 0 });
	waveB4.entries.push_back({ EnemyType::MEDIUM_PLANE, 5, 2.0f, 1 });
	waveB4.entries.push_back({ EnemyType::HEAVY_PLANE, 4, 3.0f, 1 });
	waveB4.entries.push_back({ EnemyType::SOLDIER, 5, 0.4f, 0 });
	waveB4.entries.push_back({ EnemyType::HEAVY_TANK, 3, 5.0f, 0 });
	waveB4.entries.push_back({ EnemyType::HEAVY_PLANE, 3, 3.0f, 1 });
	waveB4.entries.push_back({ EnemyType::LIGHT_TANK, 5, 1.8f, 1 });
	m2.waves.push_back(waveB4);
	Wave waveB5;
	waveB5.waveNumber = 5;
	waveB5.entries.push_back({ EnemyType::SOLDIER, 20, 0.2f, 0 });
	waveB5.entries.push_back({ EnemyType::LIGHT_TANK, 5, 1.5f, 1 });
	waveB5.entries.push_back({ EnemyType::SOLDIER, 30, 0.2f, 1 });
	waveB5.entries.push_back({ EnemyType::MEDIUM_TANK, 5, 2.5f, 0 });
    waveB5.entries.push_back({ EnemyType::LIGHT_PLANE, 5, 1.5f, 1 });
	waveB5.entries.push_back({ EnemyType::SOLDIER, 10, 0.2f, 0 });
	waveB5.entries.push_back({ EnemyType::HEAVY_TANK, 4, 3.5f, 1 });
	waveB5.entries.push_back({ EnemyType::MEDIUM_PLANE, 4, 2.0f, 0 });
	waveB5.entries.push_back({ EnemyType::SOLDIER, 10, 0.2f, 1 });
	waveB5.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 3.2f, 1 });
	waveB5.entries.push_back({ EnemyType::HEAVY_TANK, 4, 3.5f, 0 });
	waveB5.entries.push_back({ EnemyType::LIGHT_PLANE, 20, 2.0f, 0 });
	m2.waves.push_back(waveB5);
	availableMaps.push_back(m2);
    

	//Operacja Barbarossa
    MapInfo m3;
	m3.mapFile = "maps/map3.txt";
	m3.enemyTexturePrefix = "USSR";
    m3.playerCoins = 1000;
    m3.Soundtracknumber = 4;
	Wave waveC1;
    waveC1.waveNumber = 1;
    waveC1.entries.push_back({ EnemyType::SOLDIER, 5, 0.2f, 0 });
    waveC1.entries.push_back({ EnemyType::LIGHT_TANK, 3, 1.0f, 1 });
    waveC1.entries.push_back({ EnemyType::LIGHT_TANK, 3, 1.3f, 0 });
	waveC1.entries.push_back({ EnemyType::SOLDIER, 10, 0.2f, 1 });
    waveC1.entries.push_back({ EnemyType::LIGHT_PLANE, 2, 1.0f, 0 });
    waveC1.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.0f, 0 });
    waveC1.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 1.6f, 1 });
	waveC1.entries.push_back({ EnemyType::MEDIUM_TANK, 2, 2.0f, 0 });
	m3.waves.push_back(waveC1);
	Wave waveC2;
	waveC2.waveNumber = 2;
	waveC2.entries.push_back({ EnemyType::SOLDIER, 10, 0.2f, 0 });
	waveC2.entries.push_back({ EnemyType::LIGHT_TANK, 4, 1.0f, 1 });
	waveC2.entries.push_back({ EnemyType::SOLDIER, 10, 0.4f, 1 });
	waveC2.entries.push_back({ EnemyType::MEDIUM_TANK, 4, 1.3f, 0 });
	waveC2.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 0.4f, 0 });
	waveC2.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 1.6f, 1 });
	waveC2.entries.push_back({ EnemyType::HEAVY_TANK, 3, 2.0f, 0 });
	waveC2.entries.push_back({ EnemyType::HEAVY_PLANE, 3, 2.0f, 1 });
	waveC2.entries.push_back({ EnemyType::SOLDIER, 10, 0.2f, 0 });
	waveC2.entries.push_back({ EnemyType::LIGHT_TANK, 10, 1.0f, 1 });
	m3.waves.push_back(waveC2);
	Wave waveC3;
	waveC3.waveNumber = 3;
	waveC3.entries.push_back({ EnemyType::SOLDIER, 20, 0.2f, 0 });
	waveC3.entries.push_back({ EnemyType::LIGHT_TANK, 5, 1.0f, 1 });
	waveC3.entries.push_back({ EnemyType::MEDIUM_TANK, 5, 1.5f, 0 });
	waveC3.entries.push_back({ EnemyType::LIGHT_PLANE, 5, 0.7f, 1 });
	waveC3.entries.push_back({ EnemyType::MEDIUM_PLANE, 5, 1.0f, 0 });
	waveC3.entries.push_back({ EnemyType::HEAVY_TANK, 4, 2.5f, 1 });
	waveC3.entries.push_back({ EnemyType::HEAVY_PLANE, 4, 1.5f, 1});
	waveC3.entries.push_back({ EnemyType::SOLDIER, 20, 0.2f, 1 });
	waveC3.entries.push_back({ EnemyType::HEAVY_TANK, 5, 2.5f, 0 });
	waveC3.entries.push_back({ EnemyType::MEDIUM_PLANE, 5, 1.5f, 0 });
	waveC3.entries.push_back({ EnemyType::MEDIUM_TANK, 7, 2.0f, 1 });
	m3.waves.push_back(waveC3);
	Wave waveC4;
	waveC4.waveNumber = 4;
	waveC4.entries.push_back({ EnemyType::SOLDIER, 30, 0.2f, 0 });
	waveC4.entries.push_back({ EnemyType::LIGHT_TANK, 10, 1.0f, 1 });
	waveC4.entries.push_back({ EnemyType::MEDIUM_TANK, 10, 1.5f, 0 });
	waveC4.entries.push_back({ EnemyType::LIGHT_PLANE, 10, 0.8f, 1 });
	waveC4.entries.push_back({ EnemyType::MEDIUM_PLANE, 8, 1.5f, 0 });
	waveC4.entries.push_back({ EnemyType::HEAVY_TANK, 5, 2.5f, 1 });
	waveC4.entries.push_back({ EnemyType::HEAVY_PLANE, 5, 1.5f, 1 });
	waveC4.entries.push_back({ EnemyType::SOLDIER, 30, 0.2f, 1 });
	waveC4.entries.push_back({ EnemyType::HEAVY_TANK, 5, 2.5f, 0 });
	waveC4.entries.push_back({ EnemyType::HEAVY_PLANE, 6, 1.5f, 0 });
	m3.waves.push_back(waveC4);
	Wave waveC5;
	waveC5.waveNumber = 5;
	waveC5.entries.push_back({ EnemyType::SOLDIER, 40, 0.2f, 0 });
	waveC5.entries.push_back({ EnemyType::LIGHT_TANK, 10, 1.0f, 1 });
	waveC5.entries.push_back({ EnemyType::MEDIUM_TANK, 10, 1.5f, 0 });
	waveC5.entries.push_back({ EnemyType::HEAVY_TANK, 8, 2.5f, 1 });
	waveC5.entries.push_back({ EnemyType::LIGHT_PLANE, 10, 0.4f, 0 });
	waveC5.entries.push_back({ EnemyType::MEDIUM_PLANE, 10, 2.5f, 1 });
	waveC5.entries.push_back({ EnemyType::HEAVY_PLANE, 8, 1.5f, 0 });
	waveC5.entries.push_back({ EnemyType::SOLDIER, 100, 0.2f, 1 });
	m3.waves.push_back(waveC5);
	availableMaps.push_back(m3);

	//D-Day
    MapInfo m4;
    m4.mapFile = "maps/map4.txt";
    m4.enemyTexturePrefix = "USA";
	m4.playerCoins = 1200;
    m4.Soundtracknumber = 3;
    Wave waveD1;
    waveD1.waveNumber = 1;
    waveD1.entries.push_back({ EnemyType::SOLDIER, 10, 0.3f, 0 });
    waveD1.entries.push_back({ EnemyType::LIGHT_TANK, 3, 2.0f, 1 });
    waveD1.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 2.3f, 0 });
    waveD1.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 3.0f, 1 });
    waveD1.entries.push_back({ EnemyType::SOLDIER, 10, 0.3f, 1 });
    waveD1.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 2.0f, 0 });
    waveD1.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 2.6f, 1 });
	waveD1.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 3.0f, 0 });
    waveD1.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 3.0f, 0 });
    m4.waves.push_back(waveD1);
	Wave waveD2;
	waveD2.waveNumber = 2;
	waveD2.entries.push_back({ EnemyType::SOLDIER, 10, 0.7f, 0 });
	waveD2.entries.push_back({ EnemyType::LIGHT_TANK, 5, 1.5f, 1 });
	waveD2.entries.push_back({ EnemyType::MEDIUM_TANK, 5, 2.5f, 0 });
	waveD2.entries.push_back({ EnemyType::HEAVY_TANK, 3, 3.5f, 1 });
	waveD2.entries.push_back({ EnemyType::LIGHT_PLANE, 5, 1.5f, 0 });
	waveD2.entries.push_back({ EnemyType::MEDIUM_PLANE, 5, 2.5f, 1 });
	waveD2.entries.push_back({ EnemyType::HEAVY_PLANE, 3, 3.5f, 0 });
	waveD2.entries.push_back({ EnemyType::SOLDIER, 10, 0.7f, 1 });
	waveD2.entries.push_back({ EnemyType::LIGHT_PLANE, 10, 1.7f, 1 });
	waveD2.entries.push_back({ EnemyType::LIGHT_TANK, 10, 2.5f, 0 });
    m4.waves.push_back(waveD2);
	Wave waveD3;
	waveD3.waveNumber = 3;
	waveD3.entries.push_back({ EnemyType::SOLDIER, 20, 0.4f, 0 });
	waveD3.entries.push_back({ EnemyType::LIGHT_TANK, 7, 2.0f, 1 });
	waveD3.entries.push_back({ EnemyType::MEDIUM_PLANE, 4, 2.5f, 1 });
	waveD3.entries.push_back({ EnemyType::MEDIUM_TANK, 7, 3.0f, 0 });
	waveD3.entries.push_back({ EnemyType::HEAVY_TANK, 5, 4.0f, 1 });
    waveD3.entries.push_back({ EnemyType::SOLDIER, 20, 0.4f, 1 });
	waveD3.entries.push_back({ EnemyType::LIGHT_PLANE, 7, 1.5f, 0 });
	waveD3.entries.push_back({ EnemyType::MEDIUM_PLANE, 7, 2.5f, 1 });
	waveD3.entries.push_back({ EnemyType::HEAVY_PLANE, 5, 3.5f, 0 });
	waveD3.entries.push_back({ EnemyType::SOLDIER, 20, 0.4f, 1 });
	waveD3.entries.push_back({ EnemyType::HEAVY_TANK, 7, 4.5f, 0 });
	waveD3.entries.push_back({ EnemyType::LIGHT_TANK, 12, 8.0f, 0 });
    m4.waves.push_back(waveD3);
	Wave waveD4;
	waveD4.waveNumber = 4;
	waveD4.entries.push_back({ EnemyType::SOLDIER, 30, 0.3f, 0 });
    waveD4.entries.push_back({ EnemyType::LIGHT_PLANE, 10, 1.5f, 0 });
	waveD4.entries.push_back({ EnemyType::LIGHT_TANK, 10, 2.0f, 1 });
	waveD4.entries.push_back({ EnemyType::LIGHT_TANK, 10, 2.0f, 0 });
    waveD4.entries.push_back({ EnemyType::HEAVY_PLANE, 7, 3.5f, 1 });
	waveD4.entries.push_back({ EnemyType::MEDIUM_TANK, 10, 5.0f, 0 });
	waveD4.entries.push_back({ EnemyType::HEAVY_TANK, 7, 9.0f, 0 });
	waveD4.entries.push_back({ EnemyType::LIGHT_PLANE, 10, 1.5f, 0 });
	waveD4.entries.push_back({ EnemyType::MEDIUM_PLANE, 10, 2.5f, 1 });
	waveD4.entries.push_back({ EnemyType::HEAVY_PLANE, 7, 3.5f, 0 });
	waveD4.entries.push_back({ EnemyType::SOLDIER, 30, 0.3f, 1 });
    waveD4.entries.push_back({ EnemyType::HEAVY_TANK, 10, 9.0f, 1 });
	m4.waves.push_back(waveD4); 
	Wave waveD5;
	waveD5.waveNumber = 5;
	waveD5.entries.push_back({ EnemyType::SOLDIER, 50, 0.2f, 0 });
	waveD5.entries.push_back({ EnemyType::SOLDIER, 50, 0.2f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
	m4.waves.push_back(waveD5);
    availableMaps.push_back(m4);

	//Przedmieœcia Stalingradu
    MapInfo m5;
    m5.mapFile = "maps/map5.txt";
    m5.enemyTexturePrefix = "GER";
    m5.Soundtracknumber = 4;
    m5.playerCoins = 1500;
    Wave waveE1;
    waveE1.waveNumber = 1;
    waveE1.entries.push_back({ EnemyType::SOLDIER, 5, 0.9f, 0 });
    waveE1.entries.push_back({ EnemyType::LIGHT_TANK, 3, 2.0f, 1 });
    waveE1.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 2.3f, 0 });
    waveE1.entries.push_back({ EnemyType::HEAVY_TANK, 3, 3.0f, 1 });
    waveE1.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 2.0f, 0 });
    waveE1.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 2.6f, 1 });
    waveE1.entries.push_back({ EnemyType::HEAVY_PLANE, 3, 3.0f, 0 });
	m5.waves.push_back(waveE1);
	Wave waveE2;   
	waveE2.waveNumber = 2;
	waveE2.entries.push_back({ EnemyType::SOLDIER, 10, 0.7f, 0 });
	waveE2.entries.push_back({ EnemyType::LIGHT_TANK, 5, 1.5f, 1 });
	waveE2.entries.push_back({ EnemyType::LIGHT_TANK, 7, 1.5f, 0 });
	waveE2.entries.push_back({ EnemyType::LIGHT_PLANE, 5, 1.0f, 0 });
	waveE2.entries.push_back({ EnemyType::MEDIUM_TANK, 5, 2.5f, 0 });
	waveE2.entries.push_back({ EnemyType::LIGHT_PLANE, 5, 1.5f, 0 });
	waveE2.entries.push_back({ EnemyType::MEDIUM_PLANE, 5, 2.5f, 1 });
	waveE2.entries.push_back({ EnemyType::HEAVY_PLANE, 3, 3.5f, 0 });
    waveE2.entries.push_back({ EnemyType::HEAVY_TANK, 3, 3.5f, 1 });
    waveE2.entries.push_back({ EnemyType::HEAVY_TANK, 3, 3.5f, 0 });
	m5.waves.push_back(waveE2);
	Wave waveE3;
	waveE3.waveNumber = 3;
    waveE3.entries.push_back({ EnemyType::HEAVY_PLANE, 2, 3.5f, 1 });
	waveE3.entries.push_back({ EnemyType::SOLDIER, 20, 0.4f, 0 });
    waveE3.entries.push_back({ EnemyType::LIGHT_PLANE, 7, 1.5f, 0 });
	waveE3.entries.push_back({ EnemyType::LIGHT_TANK, 7, 2.0f, 1 });
    waveE3.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 3.0f, 0 });
	waveE3.entries.push_back({ EnemyType::MEDIUM_PLANE, 4, 2.5f, 1 });
	waveE3.entries.push_back({ EnemyType::MEDIUM_TANK, 7, 3.0f, 0 });
	waveE3.entries.push_back({ EnemyType::HEAVY_TANK, 2, 4.0f, 1 });
	waveE3.entries.push_back({ EnemyType::SOLDIER, 20, 0.4f, 1 });
	waveE3.entries.push_back({ EnemyType::LIGHT_PLANE, 7, 1.5f, 0 });
	waveE3.entries.push_back({ EnemyType::MEDIUM_PLANE, 7, 2.5f, 1 });
    waveE3.entries.push_back({ EnemyType::MEDIUM_TANK, 5, 4.0f, 1 });
	waveE3.entries.push_back({ EnemyType::HEAVY_PLANE, 5, 3.5f, 0 });
	waveE3.entries.push_back({ EnemyType::SOLDIER, 20, 0.4f, 1 });
    waveE3.entries.push_back({ EnemyType::LIGHT_TANK, 7, 2.0f, 0 });
	waveE3.entries.push_back({ EnemyType::HEAVY_TANK, 4, 4.5f, 0 });
	m5.waves.push_back(waveE3);
	Wave waveE4;
	waveE4.waveNumber = 4;
	waveE4.entries.push_back({ EnemyType::SOLDIER, 30, 0.3f, 0 });
	waveE4.entries.push_back({ EnemyType::LIGHT_PLANE, 10, 1.5f, 0 });
	waveE4.entries.push_back({ EnemyType::LIGHT_TANK, 15, 3.0f, 1 });
	waveE4.entries.push_back({ EnemyType::LIGHT_TANK, 15, 3.0f, 0 });
	waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 7, 2.5f, 1 });
	waveE4.entries.push_back({ EnemyType::MEDIUM_TANK, 10, 5.0f, 0 });
	waveE4.entries.push_back({ EnemyType::HEAVY_TANK, 7, 9.0f, 0 });
	waveE4.entries.push_back({ EnemyType::LIGHT_PLANE, 10, 1.5f, 0 });
	waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 10, 2.5f, 1 });
	waveE4.entries.push_back({ EnemyType::HEAVY_PLANE, 7, 3.5f, 0 });
	waveE4.entries.push_back({ EnemyType::SOLDIER, 30, 0.3f, 1 });
	waveE4.entries.push_back({ EnemyType::HEAVY_TANK, 10, 9.0f, 1 });
	m5.waves.push_back(waveE4);
	Wave waveE5;
	waveE5.waveNumber = 5;
	waveE5.entries.push_back({ EnemyType::SOLDIER, 40, 0.2f, 0 });
	waveE5.entries.push_back({ EnemyType::LIGHT_TANK, 10, 2.0f, 1 });
	waveE5.entries.push_back({ EnemyType::MEDIUM_PLANE, 10, 2.5f, 0 });
	waveE5.entries.push_back({ EnemyType::SOLDIER, 40, 0.2f, 1 });
	waveE5.entries.push_back({ EnemyType::HEAVY_TANK, 5, 3.0f, 1 });
	waveE5.entries.push_back({ EnemyType::HEAVY_PLANE, 5, 3.5f, 0 });
	waveE5.entries.push_back({ EnemyType::LIGHT_PLANE, 20, 2.2f, 0 });
	waveE5.entries.push_back({ EnemyType::MEDIUM_TANK, 10, 2.5f, 0 });
	waveE5.entries.push_back({ EnemyType::LIGHT_TANK, 20, 2.0f, 1 });
	waveE5.entries.push_back({ EnemyType::HEAVY_TANK, 8, 8.0f, 1 });
	m5.waves.push_back(waveE5);
	Wave waveE6;
	waveE6.waveNumber = 6;
	waveE6.entries.push_back({ EnemyType::SOLDIER, 20, 0.2f, 0 });
	waveE6.entries.push_back({ EnemyType::SOLDIER, 20, 0.2f, 1 });
	waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 5.0f, 1 });
	waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 5.0f, 0 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 5.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 5.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 5.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 5.0f, 0 });
    waveE6.entries.push_back({ EnemyType::LIGHT_PLANE, 9, 2.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 5.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 5.0f, 0 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 5.0f, 0 });
    waveE6.entries.push_back({ EnemyType::LIGHT_TANK, 6, 2.0f, 0 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 5.0f, 0 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 5.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 5.0f, 0 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 5.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 5.0f, 1 });
    m5.waves.push_back(waveE6);
	availableMaps.push_back(m5);

    // Load selected map
    loadMapByIndex(selectedMapIndex);

    // HUD texture
    const std::string hudPath = "assets/ui/hud.png";
    if (!hudTexture.loadFromFile(hudPath)) {
        std::cerr << "Nie mozna za³adowaæ HUD: " << hudPath << '\n';
        hudSprite.reset();
    } else {
        hudSprite = std::make_unique<sf::Sprite>(hudTexture);
        hudSprite->setPosition({ static_cast<float>(window.getSize().x) - static_cast<float>(hudTexture.getSize().x) - 10.0f, 10.0f });
    }

	// Stats table texture
    if (!statsTableTexture.loadFromFile("assets/ui/stats_table.png")) {
        std::cerr << "Nie mozna za³adowaæ tabeli statystyk: assets/ui/stats_table.png" << std::endl;
	}

    // Level icons textures
    if (!level1Texture.loadFromFile("assets/towers/level1.png")) {
        std::cerr << "Nie mozna zaladowac tekstury poziomu 1: assets/towers/level1.png" << std::endl;
    }
    if (!level2Texture.loadFromFile("assets/towers/level2.png")) {
        std::cerr << "Nie mozna zaladowac tekstury poziomu 2: assets/towers/level2.png" << std::endl;
    }
    if (!level3Texture.loadFromFile("assets/towers/level3.png")) {
        std::cerr << "Nie mozna zaladowac tekstury poziomu 3: assets/towers/level3.png" << std::endl;
    }
    if (!level4Texture.loadFromFile("assets/towers/level4.png")) {
        std::cerr << "Nie mozna zaladowac tekstury poziomu 4: assets/towers/level4.png" << std::endl;
	}   

    // load bullet textures (best effort)
    if(!bulletTextures[TowerType::INFANTRY_POST].loadFromFile("assets/bullets/IP_bullet.png")){
        std::cerr << "Nie mozna za³adowaæ tekstury pocisku: assets/bullets/IP_bullet.png" << std::endl;
	}
    if(!bulletTextures[TowerType::MACHINE_GUN].loadFromFile("assets/bullets/KM_bullet.png")){
		std::cerr << "Nie mozna za³adowaæ tekstury pocisku: assets/bullets/KM_bullet.png" << std::endl;
	}
    if (!bulletTextures[TowerType::ARTILLERY].loadFromFile("assets/bullets/ART_bullet.png")) {
		std::cerr << "Nie mozna za³adowaæ tekstury pocisku: assets/bullets/ART_bullet.png" << std::endl;
    }
    if (!bulletTextures[TowerType::AA_GUN].loadFromFile("assets/bullets/AA_bullet.png")) {
		std::cerr << "Nie mozna za³adowaæ tekstury pocisku: assets/bullets/AA_bullet.png" << std::endl;
    }

    //HP text
    if (!font.openFromFile("assets/font/SFC_Rimowa.ttf")) {
        std::cerr << "Nie mo¿na za³adowaæ czcionki: assets/font/SFC_Rimowa.ttf" << std::endl;
        font = sf::Font(); // U¿yj domyœlnej czcionki SFML
    }
    if (!fontForText.openFromFile("assets/font/montserrat.semibold.ttf")) {
        std::cerr << "Nie mo¿na za³adowaæ czcionki: assets/font/SFC_Rimowa.ttf" << std::endl;
        font = sf::Font(); // U¿yj domyœlnej czcionki SFML
    }
	//HP text
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

	// Wave text
	WaveText.setCharacterSize(40);
	WaveText.setFillColor(sf::Color::White);
	WaveText.setOutlineColor(sf::Color::Black);
	WaveText.setOutlineThickness(4.0f);
	WaveText.setPosition({ (float)window.getSize().x - 15.0f -((float)statsTableTexture.getSize().x / 2.0f), 1410.0f});
    window.display();

	// Kills text
	KillsText.setCharacterSize(40);
	KillsText.setFillColor(sf::Color::White);
	KillsText.setOutlineColor(sf::Color::Black);
	KillsText.setOutlineThickness(4.0f);
	KillsText.setPosition({ (float)window.getSize().x - ((float)statsTableTexture.getSize().x / 2.0f), 1502.0f });


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

    // Load tower icon textures
    auto loadIcon = [&](TowerType type, const std::string& file){
        sf::Texture tex;
        if (!tex.loadFromFile(file)) {
            std::cerr << "Nie mozna zaladowac ikony wiezy: " << file << std::endl;
			return;
        }
        towerIconTextures[type] = std::move(tex);
    };
    loadIcon(TowerType::AA_GUN, "assets/icons/AA_icon.png");
    loadIcon(TowerType::ARTILLERY, "assets/icons/ART_icon.png");
    loadIcon(TowerType::INFANTRY_POST, "assets/icons/IP_icon.png");
    loadIcon(TowerType::MACHINE_GUN, "assets/icons/KM_icon.png");

    // Wave button textures and sprite
    if(!buttonWaveStartTexture.loadFromFile("assets/ui/button_wave_start.png")){
        std::cerr << "Nie mozna za³adowaæ tekstury przycisku fali: assets/ui/button_wave_start.png" << std::endl;
	}
    if (!buttonWaveRunningTexture.loadFromFile("assets/ui/button_wave_running.png")) {
		std::cerr << "Nie mozna za³adowaæ tekstury przycisku fali: assets/ui/button_wave_running.png" << std::endl;
    }
    waveButtonSprite = std::make_unique<sf::Sprite>(buttonWaveStartTexture);
    // Place bottom-right with 20px margin
    {
        const auto winSize = window.getSize();
        const auto texSize = buttonWaveStartTexture.getSize();
        waveButtonSprite->setPosition({ static_cast<float>(winSize.x) - static_cast<float>(texSize.x) - statsTableTexture.getSize().x - 20.0f,
                                       static_cast<float>(winSize.y) - static_cast<float>(texSize.y) - 20.0f });
    }

    // Load action menu button textures
    if (!upgradeButtonTexture.loadFromFile("assets/ui/button_upgrade.png")) {
        std::cerr << "Nie mozna za³adowaæ assets/ui/button_upgrade.png" << std::endl;
    }
    if (!destroyButtonTexture.loadFromFile("assets/ui/button_destroy.png")) {
        std::cerr << "Nie mozna za³adowaæ assets/ui/button_destroy.png" << std::endl;
    }

    // Victory/defeat assets
    if (defeatTexture.loadFromFile("assets/ui/defeat.png")) {
        defeatSprite = std::make_unique<sf::Sprite>(defeatTexture);
        defeatSprite->setOrigin({ (float)defeatTexture.getSize().x / 2.0f, (float)defeatTexture.getSize().y / 2.0f });
        defeatSprite->setPosition({ (float)window.getSize().x / 2.0f, (float)window.getSize().y / 2.0f });
    } else {
        std::cerr << "Nie mozna za³adowaæ defeat.png (assets/ui/defeat.png)" << std::endl;
    }
    if (victoryTexture.loadFromFile("assets/ui/victory.png")) {
        victorySprite = std::make_unique<sf::Sprite>(victoryTexture);
        victorySprite->setOrigin({ (float)victoryTexture.getSize().x / 2.0f, (float)victoryTexture.getSize().y / 2.0f });
        victorySprite->setPosition({ (float)window.getSize().x / 2.0f, (float)window.getSize().y / 2.0f });
    } else {
        std::cerr << "Nie mozna za³adowaæ victory.png (assets/ui/victory.png)" << std::endl;
    }

    // End screen buttons
    if (repeatButtonTexture.loadFromFile("assets/ui/button_repeat.png")) {
        repeatButtonSprite = std::make_unique<sf::Sprite>(repeatButtonTexture);
        repeatButtonSprite->setOrigin({ (float)repeatButtonTexture.getSize().x / 2.0f, (float)repeatButtonTexture.getSize().y / 2.0f });
        repeatButtonSprite->setPosition({ (float)window.getSize().x / 2.0f + 185.0f, (float)window.getSize().y / 2.0f + 585.0f});
    } else {
        std::cerr << "Nie mozna za³adowaæ repeat.png (assets/ui/button_repeat.png)" << std::endl;
    }
    if (continueButtonTexture.loadFromFile("assets/ui/button_continue.png")) {
        continueButtonSprite = std::make_unique<sf::Sprite>(continueButtonTexture);
        continueButtonSprite->setOrigin({ (float)continueButtonTexture.getSize().x / 2.0f, (float)continueButtonTexture.getSize().y / 2.0f });
        continueButtonSprite->setPosition({ (float)window.getSize().x / 2.0f + 185.0f, (float)window.getSize().y / 2.0f + 585.0f });
    } else {
        std::cerr << "Nie mozna za³adowaæ continue.png (assets/ui/button_continue.png)" << std::endl;
    }

    // Overlay setup (dark semi-transparent rectangle as blur substitute)
    screenOverlay.setSize({ (float)window.getSize().x, (float)window.getSize().y });
    screenOverlay.setFillColor(sf::Color(0, 0, 0, 120));

    // UI: remove old rectangles (start/stop)
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
        // --- OBS£UGA MENU ---
        if (isMenuState) {
            // W menu interesuje nas tylko klikniêcie myszk¹ (do update)
            // lub wyjœcie ESC
            if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                if (keyEvent->code == sf::Keyboard::Key::Escape) {
                    window.close();
                    isRunning = false;
                }
            }
            continue; // Nie przetwarzaj reszty logiki gry (klikniêæ w wie¿e itp.)
        }

        // If game over, handle clicks only on overlay buttons
        if (gameOver) {
            if (auto mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    sf::Vector2f mouseWindowPos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
                    if (!victory) {
                        // defeat: click repeat button
                        if (repeatButtonSprite && repeatButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                            // hide end screen immediately
                            gameOver = false;
                            victory = false;
                            restartCurrentLevel();
                        }
                    } else {
                        // victory: click continue button
                        if (continueButtonSprite && continueButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                            gameOver = false;
                            victory = false;
                            goToNextMap();
                        }
                    }
                }
            }
            continue;
        }

        // Klikniêcie myszk¹
        if (auto mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseEvent->button == sf::Mouse::Button::Left) {
                sf::Vector2i pixelPos(mouseEvent->position.x, mouseEvent->position.y);
                sf::Vector2f worldPos = window.mapPixelToCoords(pixelPos);
                sf::Vector2f mouseWindowPos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));

                // If tower action menu is active, handle button clicks
                if (towerActionMenuActive) {
                    if (upgradeButtonSprite && upgradeButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                        tryUpgradeSelectedTower();
                        closeTowerActionMenu();
                        continue;
                    }
                    if (destroyButtonSprite && destroyButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                        destroySelectedTower();
                        closeTowerActionMenu();
                        continue;
                    }
                    // click elsewhere closes the menu
                    closeTowerActionMenu();
                    continue;
                }

                // Check wave button in UI
                if (waveButtonSprite && waveButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                    if (!waveManager.isActive()) {
                        waveManager.startNextWave();
                        currentWaveNumber++;
                        WaveText.setString(to_string(currentWaveNumber) + "/" + to_string(numberOfWaves));
                        waveButtonSprite->setTexture(buttonWaveRunningTexture);
                    }
                    continue;
                }

                // Check click on an existing tower to open action menu
                for (int i = 0; i < static_cast<int>(towers.size()); ++i) {
                    if (towers[i].getBounds().contains(worldPos)) {
                        openTowerActionMenu(i);
                        continue;
                    }
                }

                // Otherwise, handle building menu open
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
            case sf::Keyboard::Key::R:
                showAllRanges = !showAllRanges;
                for (auto& tower : towers) {
                    tower.toggleRangeDisplay(showAllRanges);
                }
                break;
            default:
                break;
            }
        }
    }
}

void Game::openTowerMenu(const sf::Vector2f& center) {
    towerMenuActive = true;
    Vector2f MenuCenterPos = {center.x, center.y};
    towerMenuCenter = MenuCenterPos;
    pendingBuildCenter = center; // exact center for placement
    towerMenuOptions.clear();

    // create sprites placed radially
    const float radius = 150.0f; 
    const std::vector<TowerType> types = {
        TowerType::INFANTRY_POST, TowerType::MACHINE_GUN, TowerType::ARTILLERY, TowerType::AA_GUN
    };
    const float angleStep = 2.0f * 3.1415926f / types.size();
    for (size_t i = 0; i < types.size(); ++i) {
        TowerType t = types[i];
        auto it = towerIconTextures.find(t);
        if (it == towerIconTextures.end()) continue;
        sf::Sprite spr(it->second);
        spr.setOrigin({ it->second.getSize().x / 2.0f, it->second.getSize().y / 2.0f});
        float ang = i * angleStep;
        spr.setPosition({ center.x + radius * std::cos(ang), center.y + radius * std::sin(ang) });
        spr.setScale({0.6f, 0.6f});
        towerMenuOptions.push_back({ t, spr });
    }
}

void Game::closeTowerMenu() {
    towerMenuActive = false;
    towerMenuOptions.clear();
}

void Game::openTowerActionMenu(int towerIndex) {
    // Close build menu if open
    closeTowerMenu();

    selectedTowerIndex = towerIndex;
    towerActionMenuActive = true;
    towerActionCenter = towers[towerIndex].getPosition();

    // Create/position buttons near the tower
    if (!upgradeButtonSprite)
        upgradeButtonSprite = std::make_unique<sf::Sprite>(upgradeButtonTexture);
    if (!destroyButtonSprite)
        destroyButtonSprite = std::make_unique<sf::Sprite>(destroyButtonTexture);

    const float offset = 80.0f;
    upgradeButtonSprite->setOrigin({ (float)upgradeButtonTexture.getSize().x / 2.0f, (float)upgradeButtonTexture.getSize().y / 2.0f });
    destroyButtonSprite->setOrigin({ (float)destroyButtonTexture.getSize().x / 2.0f, (float)destroyButtonTexture.getSize().y / 2.0f });

    upgradeButtonSprite->setPosition({ towerActionCenter.x - offset, towerActionCenter.y });
    destroyButtonSprite->setPosition({ towerActionCenter.x + offset, towerActionCenter.y });
}

void Game::closeTowerActionMenu() {
    towerActionMenuActive = false;
    selectedTowerIndex = -1;
}

void Game::tryUpgradeSelectedTower() {
    if (selectedTowerIndex < 0 || selectedTowerIndex >= static_cast<int>(towers.size())) return;
    Tower& t = towers[selectedTowerIndex];
    int cost = t.getUpgradeCost();
    if (!t.canUpgrade()) return;
    if (PlayerCoins < cost) {
        showLowCoinsMessage = true;
        soundManager.playNotEnoughCoinsSound();
        lowCoinsMessageTimer = 1.5f;
        lowCoinsText.setOrigin({ lowCoinsText.getLocalBounds().size.x / 2.0f, lowCoinsText.getLocalBounds().size.y / 2.0f });
        lowCoinsText.setPosition({ window.getSize().x / 2.0f, window.getSize().y / 2.0f });
        return;
    }
    if (t.getLevel() >= 4) {
        soundManager.playNotEnoughCoinsSound();
		MaxLevelText.setString("MAKSYMALNY POZIOM ULEPSZENIA");
		MaxLevelText.setCharacterSize(60);
		MaxLevelText.setFillColor(sf::Color::Red);  
		MaxLevelText.setOutlineColor(sf::Color::Black);
		MaxLevelText.setOutlineThickness(4.0f);
        // Use MaxLevelText local bounds for origin, not lowCoinsText
        MaxLevelText.setOrigin({ MaxLevelText.getLocalBounds().size.x / 2.0f, MaxLevelText.getLocalBounds().size.y / 2.0f });
        MaxLevelText.setPosition({ window.getSize().x / 2.0f, window.getSize().y / 2.0f });
        showMaxLevelMessage = true;
        // Use dedicated timer for max level message
        maxLevelMessageTimer = 1.5f;
        std::cout << "Maksymalny poziom ulepszenia\n";
		return;
    }
    PlayerCoins -= cost;
    CoinsText.setString(std::to_string(PlayerCoins));
    CoinsText.setOrigin({ CoinsText.getLocalBounds().size.x / 2.0f, CoinsText.getLocalBounds().size.y / 2.0f });
    t.upgrade();

    // Update level icon texture for this tower
    if (selectedTowerIndex >= 0 && selectedTowerIndex < static_cast<int>(levelIcons.size()) && levelIcons[selectedTowerIndex]) {
        int lvl = t.getLevel();
        if (lvl == 2) levelIcons[selectedTowerIndex]->setTexture(level2Texture);
        else if (lvl == 3) levelIcons[selectedTowerIndex]->setTexture(level3Texture);
		else if (lvl == 4) levelIcons[selectedTowerIndex]->setTexture(level4Texture);
        else levelIcons[selectedTowerIndex]->setTexture(level1Texture);
    }
}

void Game::destroySelectedTower() {
    if (selectedTowerIndex < 0 || selectedTowerIndex >= static_cast<int>(towers.size())) return;
    // Optionally refund part of cost
    PlayerCoins += towers[selectedTowerIndex].getCost() / 2;
    CoinsText.setString(std::to_string(PlayerCoins));
    CoinsText.setOrigin({ CoinsText.getLocalBounds().size.x / 2.0f, CoinsText.getLocalBounds().size.y / 2.0f });

    // Free the build area on the map
    map.freeBuildAreaAtCenter(towers[selectedTowerIndex].getPosition());

    towers.erase(towers.begin() + selectedTowerIndex);
    // remove corresponding level icon
    if (selectedTowerIndex >= 0 && selectedTowerIndex < static_cast<int>(levelIcons.size())) {
        levelIcons.erase(levelIcons.begin() + selectedTowerIndex);
    }
    selectedTowerIndex = -1;
}

void Game::handleMouseClick(const sf::Vector2f& position) {
    // If menu is open, handle selection
	soundManager.playClickSound();
    if (towerMenuActive) {
        for (const auto& opt : towerMenuOptions) {
            if (opt.sprite.getGlobalBounds().contains(position)) {
                selectedTowerType = opt.type;
                sf::Vector2f center = pendingBuildCenter;
                int cost = TOWER_CONFIGS.at(selectedTowerType).cost;
                if (PlayerCoins < cost) {
                    showLowCoinsMessage = true;
                    soundManager.playNotEnoughCoinsSound();
                    lowCoinsMessageTimer = 1.5f;
                    lowCoinsText.setOrigin({ lowCoinsText.getLocalBounds().size.x / 2.0f, lowCoinsText.getLocalBounds().size.y / 2.0f });
                    lowCoinsText.setPosition({ window.getSize().x / 2.0f, window.getSize().y / 2.0f });
                    closeTowerMenu();
                    std::cout << "Brakuje monet\n";
                    return;
                }
                // occupy area; if already occupied, block building
                if (!map.occupyBuildAreaAtCenter(center)) {
                    closeTowerMenu();
                    std::cout << "To miejsce jest zajête\n";
                    return;
                }
                PlayerCoins -= cost;
                CoinsText.setString(std::to_string(PlayerCoins));
                CoinsText.setOrigin({ CoinsText.getLocalBounds().size.x / 2.0f, CoinsText.getLocalBounds().size.y / 2.0f });
                towers.emplace_back(center.x, center.y, selectedTowerType);
                auto it = towerTextures.find(selectedTowerType);
                if (it != towerTextures.end()) {
                    towers.back().setTexture(it->second);
                    soundManager.playTowerPlaceSound();
                }
                // create level icon for this new tower
                {
                    auto icon = std::make_unique<sf::Sprite>(level1Texture);
                    icon->setOrigin({ (float)level1Texture.getSize().x / 2.0f, (float)level1Texture.getSize().y / 2.0f });
                    // position to right of tower
                    sf::Vector2f tpos = towers.back().getPosition();
                    icon->setPosition({ tpos.x , tpos.y + (TOWER_CONFIGS.at(selectedTowerType).size.y / 2.0f) + 10.0f });
                    levelIcons.push_back(std::move(icon));
                }
                towers.back().toggleRangeDisplay(showAllRanges);
                closeTowerMenu();
                return;
            }
        }
        // click outside -> close without building
        closeTowerMenu();
        return;
    }

    // Open menu if clicking buildable area without claiming immediately
    sf::Vector2f center;
    if (map.canBuildHere(position) && map.getBuildAreaCenter(position, center)) {
        openTowerMenu(center);
        return;
    }

    std::cout << "Nie mozna budowac w tym miejscu\n";
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
    if (isMenuState) {
        sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
        sf::Vector2f mousePos = window.mapPixelToCoords(pixelPos);
        bool mousePressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);

        int action = menu->update(mousePos, mousePressed);

        // Klikniêcie GRAJ (w g³ównym menu)
        if (action == 1) {
            // 1. Rysujemy wciœniêty przycisk
            window.clear(sf::Color::Black);
            menu->draw(window);
            window.display();

            // 2. Czekamy
            sf::sleep(sf::milliseconds(200));

            // 3. Zmieniamy stan MENU na wybór poziomów (NIE startujemy jeszcze gry!)
            menu->switchToLevelSelect();

            // Czekamy chwilê, ¿eby puszczenie myszki nie kliknê³o od razu w mapê
            sf::sleep(sf::milliseconds(200));
        }

        // Klikniêcie w MAPÊ (w menu wyboru poziomów)
        // Kody od 100 w górê to wybór mapy
        else if (action >= 100) {
            int mapIndex = action - 100; // 100 -> 0, 101 -> 1 itd.

            // 1. Rysujemy wciœniêt¹ mapê (feedback wizualny)
            window.clear(sf::Color::Black);
            menu->draw(window);
            window.display();
            sf::sleep(sf::milliseconds(200));

            // 2. £adujemy wybran¹ mapê
            loadMapByIndex(mapIndex);

            // 3. Startujemy grê
            isMenuState = false;
        }

        return; // Koniec update dla menu
    }
    // Aktualizacja fal i generowanych przeciwników
    const auto& path1 = map.getPathPoints();
    const auto& path2 = map.getPathPoints2();

    waveManager.update(deltaTime, enemies, path1, path2, currentMapEnemyPrefix);

    // If wave finished, restore button texture
    if (waveButtonSprite && !waveManager.isActive()) {
        waveButtonSprite->setTexture(buttonWaveStartTexture);
    }

    // Defeat check
    if (PlayerHP <= 0 && !gameOver) {
        triggerDefeat();
    }

    // Victory check: all waves started and finished, and no enemies left and no spawns pending
    if (!gameOver) {
        bool wavesDone = (currentWaveNumber >= numberOfWaves);
        bool noEnemies = enemies.empty();
        bool spawnsIdle = !waveManager.isActive();
        if (wavesDone && noEnemies && spawnsIdle) {
            triggerVictory();
        }
    }

    // Tower shooting
    for (auto& tower : towers) {
        tower.tickCooldown(deltaTime);
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
            if (tower.getType() == TowerType::AA_GUN) {
                soundManager.playAAshootSound();
            } else if (tower.getType() == TowerType::ARTILLERY) {
                soundManager.playARTshootSound();
            } else if (tower.getType() == TowerType::MACHINE_GUN) {
                soundManager.playMGshootSound();
            } else if (tower.getType() == TowerType::INFANTRY_POST) {
                soundManager.playIPshootSound();
			}
            projectiles.push_back(std::move(proj));
            tower.resetCooldown();
        } else if (target) {
            sf::Vector2f dir = target->getPosition() - tpos;
            tower.rotateToDirection(dir);
        }
    }

    // Update projectiles
    for (auto it = projectiles.begin(); it != projectiles.end();) {
        auto& p = *it;
        p->update(deltaTime);
        bool removed = false;
        if (!p->isAlive()) {
            it = projectiles.erase(it);
            continue;
        }
        for (auto& ePtr : enemies) {
            if (!ePtr) continue;
            float dx = ePtr->getPosition().x - p->getPosition().x;
            float dy = ePtr->getPosition().y - p->getPosition().y;
            float dist2 = dx*dx + dy*dy;
            if (dist2 < 20.0f * 20.0f) {
                ePtr->takeDamage(p->getDamage());
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
            if (e->getHealth() > 0) {
                applyDamageToPlayer(e->getType(), PlayerHP, HPText);
            } else {
                PlayerCoins += e->getReward();
                CoinsText.setString(std::to_string(PlayerCoins));
                CoinsText.setOrigin({ CoinsText.getLocalBounds().size.x / 2.0f, CoinsText.getLocalBounds().size.y / 2.0f });
				KillsCount++;
				KillsText.setString(to_string(KillsCount));
            }
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
    if (showMaxLevelMessage) {
        maxLevelMessageTimer -= deltaTime;
        if (maxLevelMessageTimer <= 0.0f) {
            showMaxLevelMessage = false;
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
    // ZMIEÑ: sf::Color::Black na sf::Color::Magenta
    window.clear(sf::Color::Magenta);

    // --- RYSOWANIE MENU ---
    if (isMenuState) {
        menu->draw(window);
        window.display();
        return;
    }
    const sf::Texture& bgTex = map.getBackgroundTexture();
    sf::Sprite backgroundSprite(bgTex);
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
        sf::Sprite hudSpriteLocal(hudTexture);
        hudSpriteLocal.setPosition({ (float)window.getSize().x - (float)hudTexture.getSize().x, 0.0f});
        window.draw(hudSpriteLocal);
    }
	sf::Sprite statsTableSpriteLocal(statsTableTexture);
    statsTableSpriteLocal.setOrigin({ (float)statsTableTexture.getSize().x / 2.0f, (float)statsTableTexture.getSize().y / 2.0f });
	statsTableSpriteLocal.setPosition({ (float)window.getSize().x - ((float)statsTableTexture.getSize().x / 2.0f), (float)window.getSize().y - ((float)statsTableTexture.getSize().y / 2.0f)});

	window.draw(statsTableSpriteLocal);
	window.draw(WaveText);

    // Draw wave button
    if (waveButtonSprite)
        window.draw(*waveButtonSprite);

    // Draw level icons next to towers
    for (size_t i = 0; i < towers.size() && i < levelIcons.size(); ++i) {
        if (!levelIcons[i]) continue;
        // keep icon aligned to tower position
        sf::Vector2f tpos = towers[i].getPosition();
        float offsetY = TOWER_CONFIGS.at(towers[i].getType()).size.y / 2.0f + 8.0f;
        levelIcons[i]->setPosition({ tpos.x, tpos.y + offsetY });
        window.draw(*levelIcons[i]);
    }

    window.draw(HPText);
    window.draw(CoinsText);
	window.draw(KillsText);

    // draw low coins text if active
    if (showLowCoinsMessage) {
        window.draw(lowCoinsText);
    }

    if (showMaxLevelMessage) {
        window.draw(MaxLevelText);
	}

    // draw radial tower menu
    if (towerMenuActive) {
        // a subtle circle background
        sf::CircleShape bgCircle(80.0f);
        bgCircle.setOrigin({80.0f, 80.0f});
        bgCircle.setPosition(towerMenuCenter);
        bgCircle.setFillColor(sf::Color(0,0,0,0)); //WY£¥CZONE
        bgCircle.setOutlineColor(sf::Color(255,255,255,0)); //WY£¥CZONE
        bgCircle.setOutlineThickness(2.0f);
        window.draw(bgCircle);
        for (const auto& opt : towerMenuOptions) {
            window.draw(opt.sprite);
        }
    }

    // Draw tower action menu buttons if active
    if (towerActionMenuActive) {
        if (upgradeButtonSprite)
            window.draw(*upgradeButtonSprite);
        if (destroyButtonSprite)
            window.draw(*destroyButtonSprite);
    }

    // End screens
    if (gameOver) {
        // Draw dark overlay first, then end screen sprite above
        if ((!victory && defeatSprite) || (victory && victorySprite)) {
            window.draw(screenOverlay);
        }
        if (!victory && defeatSprite) {
            window.draw(*defeatSprite);
            if (repeatButtonSprite)
                window.draw(*repeatButtonSprite);
        } else if (victory && victorySprite) {
            window.draw(*victorySprite);
            if (continueButtonSprite)
                window.draw(*continueButtonSprite);
        }
    }

    window.display();
}

void Game::triggerDefeat() {
    gameOver = true;
    victory = false;
}

void Game::triggerVictory() {
    gameOver = true;
    victory = true;
}

void Game::restartCurrentLevel() {
    // reset towers, enemies, projectiles
    towers.clear();
    enemies.clear();
    projectiles.clear();
    levelIcons.clear();

    // Clear map paths/build areas to avoid duplicates
    map.clearPathsAndBuildAreas();

    // Reset player HP and HUD
    PlayerHP = 100;
    HPText.setString(std::to_string(PlayerHP) + "/100");
    HPText.setOrigin({ HPText.getLocalBounds().size.x / 2.0f, HPText.getLocalBounds().size.y / 2.0f });

    // reload current map
    loadMapByIndex(selectedMapIndex);

    // restore wave button texture
    if (waveButtonSprite)
        waveButtonSprite->setTexture(buttonWaveStartTexture);
}

void Game::goToNextMap() {
    int nextIndex = selectedMapIndex + 1;
    if (nextIndex >= static_cast<int>(availableMaps.size())) {
        nextIndex = 0; // loop back or stay; here we loop to first
    }
    towers.clear();
    enemies.clear();
    projectiles.clear();
    levelIcons.clear();

    // Clear map paths/build areas to avoid duplicates
    map.clearPathsAndBuildAreas();

    // Reset player HP and HUD
    PlayerHP = 100;
    HPText.setString(std::to_string(PlayerHP) + "/100");
    HPText.setOrigin({ HPText.getLocalBounds().size.x / 2.0f, HPText.getLocalBounds().size.y / 2.0f });

    loadMapByIndex(nextIndex);

    // restore wave button texture
    if (waveButtonSprite)
        waveButtonSprite->setTexture(buttonWaveStartTexture);
}

