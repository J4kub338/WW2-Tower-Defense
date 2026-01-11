#include "Game.hpp"
#include <iostream>
#include <cmath>
#include <unordered_set>

namespace {
    void logOnce(const char* msg) {
        static std::unordered_set<std::string> seen;
        if (seen.insert(msg).second) {
            std::cerr << msg << std::endl;
        }
    }

    // Track currently playing menu music (per process)
    bool g_menuMusicPlaying = false;
}

static std::unique_ptr<Projectile> makeProjectileForTower(TowerType type, const sf::Vector2f& pos, const sf::Vector2f& dir,
                                                         const std::unordered_map<TowerType, sf::Texture>& bulletTextures,
                                                         int damage, float maxRange,
                                                         float explosionRadius = 0.0f, float edgeDamageMultiplier = 1.0f) {
    const sf::Texture* tex = nullptr;
    auto it = bulletTextures.find(type);
    if (it != bulletTextures.end()) tex = &it->second;
    float speed = 400.0f;
    switch (type) {
    case TowerType::INFANTRY_POST: speed = 900.0f; break;
    case TowerType::MACHINE_GUN: speed = 1000.0f; break;
    case TowerType::ARTILLERY: speed = 1000.0f; break;
    case TowerType::AA_GUN: speed = 900.0f; break;
    }
    return std::make_unique<Projectile>(pos, dir, speed, damage, tex, maxRange, explosionRadius, edgeDamageMultiplier);
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
    soundManager.playBackground("music/track" + std::to_string(info.Soundtracknumber) + ".mp3", true, 20.0f);

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
	
    MapNameText.setString(info.mapName);
	MapNameTextBigger.setString(info.mapName);
    MapNameTextBigger.setOrigin({ MapNameTextBigger.getLocalBounds().size.x / 2.0f, MapNameTextBigger.getLocalBounds().size.y / 2.0f });

    gameOver = false;
    victory = false;

    // For now we only store enemy texture prefix in map info and pass it when spawning enemies via WaveManager.
    // WaveManager currently creates enemies without custom texture names. A full implementation would propagate texture overrides when creating Enemy instances.
}

Game::Game()
    : window(sf::VideoMode({ 2580, 1600 }), "Tower Defense - WWII", State::Fullscreen),
      selectedTowerType(TowerType::INFANTRY_POST),
      showAllRanges(false),
      isRunning(true),
      HPText(font),
      CoinsText(font),
      lowCoinsText(font),
	  WaveText(font),
	  KillsText(font),
	  MaxLevelText(font),
      upgradeCostText(font),
      MapNameText(font),
	  MapNameTextBigger(font)
{
    std::cerr << "[BOOT] Game::Game() start\n";

    // Setup available maps (basic example)
    //Kampania wrzeœniowa
    MapInfo m1;
	m1.playerCoins = 600;
    m1.mapFile = "maps/map1.txt";
    m1.enemyTexturePrefix = "GER";
	m1.Soundtracknumber = 3;
	m1.mapName = "Kampania wrzesniowa";
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
	m2.mapName = "Afryka Polnocna";
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
	m3.mapName = "Operacja Barbarossa";
	Wave waveC1;
    waveC1.waveNumber = 1;
    waveC1.entries.push_back({ EnemyType::SOLDIER, 10, 0.2f, 0 });
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
	m4.playerCoins = 1900;
    m4.Soundtracknumber = 3;
	m4.mapName = "D-Day";
    Wave waveD1;
    waveD1.waveNumber = 1;
	waveD1.entries.push_back({ EnemyType::MINI_BOSS, 1, 0.3f, 0 });
    waveD1.entries.push_back({ EnemyType::SOLDIER, 100, 0.1f, 1 });
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
    waveD5.entries.push_back({ EnemyType::MINI_BOSS, 1, 0.3f, 0 });
    waveD5.entries.push_back({ EnemyType::MINI_BOSS, 1, 2.4f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 0 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 0 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 0 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 0});
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 0 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
    waveD5.entries.push_back({ EnemyType::HEAVY_TANK, 1, 2.0f, 0 });
    waveD5.entries.push_back({ EnemyType::HEAVY_PLANE, 1, 2.0f, 1 });
	m4.waves.push_back(waveD5);
    availableMaps.push_back(m4);

	//Przedmieœcia Stalingradu
    MapInfo m5;
    m5.mapFile = "maps/map5.txt";
    m5.enemyTexturePrefix = "GER";
    m5.Soundtracknumber = 4;
    m5.playerCoins = 15000;
	m5.mapName = "Obrzeza Stalingradu";
    Wave waveE1;
    waveE1.waveNumber = 1;
    waveE1.entries.push_back({ EnemyType::SOLDIER, 5, 0.9f, 0 });
    waveE1.entries.push_back({ EnemyType::LIGHT_TANK, 3, 1.5f, 1 });
    waveE1.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 2.0f, 0 });
    waveE1.entries.push_back({ EnemyType::HEAVY_TANK, 3, 2.5f, 1 });
    waveE1.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 1.2f, 0 });
    waveE1.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 1.8f, 1 });
    waveE1.entries.push_back({ EnemyType::HEAVY_PLANE, 3, 2.2f, 0 });
	m5.waves.push_back(waveE1);
	Wave waveE2;   
	waveE2.waveNumber = 2;
	waveE2.entries.push_back({ EnemyType::SOLDIER, 10, 0.7f, 0 });
	waveE2.entries.push_back({ EnemyType::LIGHT_TANK, 5, 1.2f, 1 });
	waveE2.entries.push_back({ EnemyType::LIGHT_TANK, 3, 1.2f, 0 });
    waveE2.entries.push_back({ EnemyType::HEAVY_TANK, 2, 1.4f, 1 });
	waveE2.entries.push_back({ EnemyType::LIGHT_PLANE, 5, 1.0f, 0 });
	waveE2.entries.push_back({ EnemyType::MEDIUM_TANK, 5, 2.0f, 0 });
	waveE2.entries.push_back({ EnemyType::LIGHT_PLANE, 5, 1.0f, 0 });
	waveE2.entries.push_back({ EnemyType::MEDIUM_PLANE, 5, 1.5f, 1 });
	waveE2.entries.push_back({ EnemyType::HEAVY_PLANE, 3, 2.0f, 0 });
    waveE2.entries.push_back({ EnemyType::HEAVY_TANK, 3, 2.3f, 1 });
    waveE2.entries.push_back({ EnemyType::HEAVY_TANK, 3, 2.3f, 0 });
	m5.waves.push_back(waveE2);
	Wave waveE3;
	waveE3.waveNumber = 3;
    waveE3.entries.push_back({ EnemyType::HEAVY_PLANE, 2, 1.0f, 1 });
	waveE3.entries.push_back({ EnemyType::SOLDIER, 20, 0.2f, 0 });
    waveE3.entries.push_back({ EnemyType::LIGHT_PLANE, 7, 1.0f, 0 });
	waveE3.entries.push_back({ EnemyType::LIGHT_TANK, 4, 1.2f, 1 });
    waveE3.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 1.6f, 1 });
    waveE3.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 1.3f, 0 });
	waveE3.entries.push_back({ EnemyType::MEDIUM_PLANE, 4, 1.0f, 1 });
	waveE3.entries.push_back({ EnemyType::MEDIUM_TANK, 7, 1.2f, 0 });
	waveE3.entries.push_back({ EnemyType::HEAVY_TANK, 2, 1.0f, 1 });
	waveE3.entries.push_back({ EnemyType::SOLDIER, 20, 0.2f, 1 });
	waveE3.entries.push_back({ EnemyType::LIGHT_PLANE, 7, 1.0f, 0 });
	waveE3.entries.push_back({ EnemyType::MEDIUM_PLANE, 7, 1.3f, 1 });
    waveE3.entries.push_back({ EnemyType::MEDIUM_TANK, 5, 1.0f, 1 });
	waveE3.entries.push_back({ EnemyType::HEAVY_PLANE, 5, 1.0f, 0 });
	waveE3.entries.push_back({ EnemyType::SOLDIER, 20, 0.2f, 1 });
    waveE3.entries.push_back({ EnemyType::LIGHT_TANK, 7, 1.4f, 0 });
	waveE3.entries.push_back({ EnemyType::HEAVY_TANK, 4, 1.5f, 0 });
	m5.waves.push_back(waveE3);
	Wave waveE4;
	waveE4.waveNumber = 4;
	waveE4.entries.push_back({ EnemyType::SOLDIER, 30, 0.2f, 0 });
	waveE4.entries.push_back({ EnemyType::LIGHT_PLANE, 10, 1.2f, 0 });
	waveE4.entries.push_back({ EnemyType::LIGHT_TANK, 5, 1.2f, 1 });
	waveE4.entries.push_back({ EnemyType::LIGHT_TANK, 5, 1.2f, 0 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 7, 1.8f, 1 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 1.9f, 1 });
    waveE4.entries.push_back({ EnemyType::HEAVY_PLANE, 5, 2.2f, 0 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 2, 0.8f, 1 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 2, 0.8f, 1 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 1.0f, 1 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 2, 0.8f, 1 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 1.0f, 1 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 2, 0.8f, 1 });
	waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 4, 1.8f, 1 });
	waveE4.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 2.0f, 0 });
	waveE4.entries.push_back({ EnemyType::HEAVY_TANK, 2, 3.0f, 0 });
	waveE4.entries.push_back({ EnemyType::LIGHT_PLANE, 4, 1.5f, 0 });
	waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 2, 0.8f, 1 });
	waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 2, 0.8f, 1 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 1.0f, 1 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 2, 0.8f, 1 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_PLANE, 2, 0.8f, 1 });
	waveE4.entries.push_back({ EnemyType::HEAVY_PLANE, 3, 2.5f, 0 });
    waveE4.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 1.9f, 1 });
	m5.waves.push_back(waveE4);
	Wave waveE5;
	waveE5.waveNumber = 5;
	waveE5.entries.push_back({ EnemyType::SOLDIER, 40, 0.2f, 0 });
	waveE5.entries.push_back({ EnemyType::LIGHT_TANK, 10, 2.0f, 1 });
	waveE5.entries.push_back({ EnemyType::MEDIUM_PLANE, 10, 2.5f, 0 });
	waveE5.entries.push_back({ EnemyType::SOLDIER, 40, 0.2f, 1 });
	waveE5.entries.push_back({ EnemyType::HEAVY_TANK, 5, 3.0f, 1 });
	waveE5.entries.push_back({ EnemyType::HEAVY_PLANE, 5, 3.5f, 0 });
	waveE5.entries.push_back({ EnemyType::LIGHT_PLANE, 20, 1.2f, 0 });
	waveE5.entries.push_back({ EnemyType::MEDIUM_TANK, 10, 2.5f, 0 });
	waveE5.entries.push_back({ EnemyType::LIGHT_TANK, 20, 2.0f, 1 });
	waveE5.entries.push_back({ EnemyType::HEAVY_TANK, 8, 8.0f, 1 });
	m5.waves.push_back(waveE5);
	Wave waveE6;
	waveE6.waveNumber = 5;
	waveE6.entries.push_back({ EnemyType::SOLDIER, 20, 0.2f, 0 });
    waveE6.entries.push_back({ EnemyType::BOSS, 1, 3.5f, 0 });
	waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 1.5f, 1 });
	waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 5, 1.0f, 0 });
    waveE6.entries.push_back({ EnemyType::BOSS, 1, 3.5f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 2.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 1.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 2.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 1.0f, 0 });
    waveE6.entries.push_back({ EnemyType::LIGHT_PLANE, 9, 0.3f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 1.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 0.5f, 0 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 7, 1.5f, 0 });
    waveE6.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 0.9f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 5, 1.0f, 0 });
    waveE6.entries.push_back({ EnemyType::MEDIUM_PLANE, 4, 0.3f, 1 });
    waveE6.entries.push_back({ EnemyType::MEDIUM_TANK, 8, 0.4f, 0 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 4, 1.5f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 1.8f, 0 });
    waveE6.entries.push_back({ EnemyType::LIGHT_TANK, 6, 1.2f, 0 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 0.5f, 0 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 2.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 0.5f, 0 });
    waveE6.entries.push_back({ EnemyType::HEAVY_TANK, 5, 1.0f, 1 });
    waveE6.entries.push_back({ EnemyType::HEAVY_PLANE, 10, 0.5f, 1 });
    m5.waves.push_back(waveE6);
	availableMaps.push_back(m5);

	//Bitwa o Berlin
    MapInfo m6;
	m6.mapFile = "maps/map6.txt";
    m6.enemyTexturePrefix = "USSR";
	m6.playerCoins = 15000;
	m6.Soundtracknumber = 3;
	m6.mapName = "Bitwa o Berlin";
    Wave waveF1;
    waveF1.waveNumber = 1;
	waveF1.entries.push_back({ EnemyType::BOSS, 1, 0.2f, 0 });
    waveF1.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 2.0f, 0 });
	waveF1.entries.push_back({ EnemyType::BOSS, 1, 2.0f, 1 });
    waveF1.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 3.0f, 0 });
    waveF1.entries.push_back({ EnemyType::BOSS, 1, 6.0f, 0 });
    waveF1.entries.push_back({ EnemyType::SOLDIER, 10, 0.3f, 0 });
    waveF1.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 2.6f, 1 });
    waveF1.entries.push_back({ EnemyType::LIGHT_TANK, 3, 2.0f, 1 });
    waveF1.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 2.3f, 0 });
    waveF1.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 2.0f, 0 });
    waveF1.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 3.0f, 1 });
    waveF1.entries.push_back({ EnemyType::SOLDIER, 10, 0.3f, 1 });
    waveF1.entries.push_back({ EnemyType::LIGHT_PLANE, 3, 2.0f, 0 });
	waveF1.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 2.6f, 1 });
    waveF1.entries.push_back({ EnemyType::MEDIUM_TANK, 3, 3.0f, 0 });
    waveF1.entries.push_back({ EnemyType::MEDIUM_PLANE, 3, 3.0f, 0 });
	m6.waves.push_back(waveF1);
	availableMaps.push_back(m6);

    // Load selected map
    loadMapByIndex(selectedMapIndex);

    // IMPORTANT: start from main menu
    gameState = GameState::MAIN_MENU;

    // Start main menu music (loop)
    soundManager.playBackground("music/main_menu_music.mp3", true, 20.0f);
    g_menuMusicPlaying = true;

    std::cerr << "[BOOT] gameState set to MAIN_MENU\n";

    std::cerr << "[BOOT] Game::Game() end\n";

    // HUD texture
    const std::string hudPath = "assets/ui/hud.png";
    if (!hudTexture.loadFromFile(hudPath)) {
        std::cerr << "Nie mozna za³adowaæ HUD: " << hudPath << '\n';
        // NOTE: we don't keep a persistent sprite; we draw from texture each frame when valid.
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

	//In game menu 
    if (!inGameMenuTexture.loadFromFile("assets/ui/pause_panel.png")) {
        std::cerr << "Nie mozna za³adowac tekstury menu w grze: assets/ui/pause_panel.png" << std::endl;
	} else {
        inGameMenuSprite = std::make_unique<sf::Sprite>(inGameMenuTexture);
        inGameMenuSprite->setOrigin({ inGameMenuTexture.getSize().x / 2.0f, inGameMenuTexture.getSize().y / 2.0f });
        inGameMenuSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f });
	}
    if (!resumeButtonTexture.loadFromFile("assets/ui/button_resume.png")) {
        std::cerr << "Nie mozna za³adowac tekstury przycisku wznowienia: assets/ui/button_resume.png" << std::endl;
    } else {
        resumeButtonSprite = std::make_unique<sf::Sprite>(resumeButtonTexture); //resume
        resumeButtonSprite->setOrigin({ resumeButtonTexture.getSize().x / 2.0f, resumeButtonTexture.getSize().y / 2.0f });
        resumeButtonSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y - 80.0f) / 2.0f - 193.0f });
	}
    if (!mainMenuButtonTexture.loadFromFile("assets/ui/button_quit.png")) {
        std::cerr << "Nie mozna za³adowac tekstury przycisku menu g³ównego: assets/ui/button_quit.png" << std::endl;
    }
    else {
        mainMenuButtonSprite = std::make_unique<sf::Sprite>(mainMenuButtonTexture); //quit
        mainMenuButtonSprite->setOrigin({ mainMenuButtonTexture.getSize().x / 2.0f, mainMenuButtonTexture.getSize().y / 2.0f });
        mainMenuButtonSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f + 148.0f });
    }
    if (!restartButtonTexture.loadFromFile("assets/ui/button_restart.png")) {
        std::cerr << "Nie mozna za³adowac tekstury przycisku restartu: assets/ui/button_restart.png" << std::endl;
    } else
    {
		restartButtonSprite = std::make_unique<sf::Sprite>(restartButtonTexture); //restart
        restartButtonSprite->setOrigin({ restartButtonTexture.getSize().x / 2.0f, restartButtonTexture.getSize().y / 2.0f });
		restartButtonSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f - 42.0f});
    }
    if (!soundOnTexture.loadFromFile("assets/ui/sfx_on.png")) {
        std::cerr << "Nie mozna za³adowac tekstury przycisku dŸwiêku w³¹czonego: assets/ui/sound_on.png" << std::endl;
	} else{
        soundButtonSprite = std::make_unique<sf::Sprite>(soundOnTexture); //sound
		soundButtonSprite->setOrigin({ soundOnTexture.getSize().x / 2.0f, soundOnTexture.getSize().y / 2.0f });
		soundButtonSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y)/ 2.0f + 371.0f });
	}
    if (!soundOffTexture.loadFromFile("assets/ui/sfx_off.png")) {
        std::cerr << "Nie mozna za³adowac tekstury przycisku dŸwiêku wy³¹czonego: assets/ui/sound_off.png" << std::endl;
	}

    // --- Guidebook / Help assets (pause menu) ---
    if (!helpButtonTexture.loadFromFile("assets/ui/button_help.png")) {
        std::cerr << "Nie mozna zaladowac: assets/ui/button_help.png" << std::endl;
    } else {
        helpButtonSprite = std::make_unique<sf::Sprite>(helpButtonTexture);
        helpButtonSprite->setOrigin({ helpButtonTexture.getSize().x / 2.0f, helpButtonTexture.getSize().y / 2.0f });
        // Below sound button (same X as other pause buttons)
        helpButtonSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f + 610.0f });
    }

    if (!guidebook1Texture.loadFromFile("assets/ui/guidebook/guidebook1.png")) {
        std::cerr << "Nie mozna zaladowac: assets/ui/guidebook/guidebook1.png" << std::endl;
    } else {
        guidebook1Texture.setSmooth(true);
    }
    if (!guidebook2Texture.loadFromFile("assets/ui/guidebook/guidebook2.png")) {
        std::cerr << "Nie mozna zaladowac: assets/ui/guidebook/guidebook2.png" << std::endl;
    } else {
        guidebook2Texture.setSmooth(true);
    }
    if (guidebook1Texture.getSize().x > 0 && guidebook1Texture.getSize().y > 0) {
        guidebookSprite = std::make_unique<sf::Sprite>(guidebook1Texture);
        guidebookSprite->setOrigin({ guidebook1Texture.getSize().x / 2.0f, guidebook1Texture.getSize().y / 2.0f });
        guidebookSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f });
    }

    if (!changePageButtonTexture.loadFromFile("assets/ui/button_change_page.png")) {
        std::cerr << "Nie mozna zaladowac: assets/ui/button_change_page.png" << std::endl;
    } else {
        changePageButtonSprite = std::make_unique<sf::Sprite>(changePageButtonTexture);
        changePageButtonSprite->setOrigin({ changePageButtonTexture.getSize().x / 2.0f, changePageButtonTexture.getSize().y / 2.0f });
        // Bottom-right corner
        const auto winSize = window.getSize();
        const auto texSize = changePageButtonTexture.getSize();
        changePageButtonSprite->setPosition({ static_cast<float>(winSize.x) - static_cast<float>(texSize.x) / 2.0f - 40.0f,
                                              static_cast<float>(winSize.y) - static_cast<float>(texSize.y) / 2.0f - 40.0f });
    }

    // UI: upgrade cost label (hidden by default; positioned when menu opens)
    upgradeCostText.setCharacterSize(28);
    upgradeCostText.setFillColor(sf::Color::White);
    upgradeCostText.setOutlineColor(sf::Color::Black);
    upgradeCostText.setOutlineThickness(3.0f);
    upgradeCostText.setString("");
    if (!backFromGuideButtonTexture.loadFromFile("assets/ui/btn_back.png")) {
        std::cerr << "Nie mozna zaladowac: assets/ui/btn_back.png" << std::endl;
    } else {
        backFromGuideButtonSprite = std::make_unique<sf::Sprite>(backFromGuideButtonTexture);
        backFromGuideButtonSprite->setOrigin({ backFromGuideButtonTexture.getSize().x / 2.0f, backFromGuideButtonTexture.getSize().y / 2.0f });
        // Bottom-left corner
        const auto texSize = backFromGuideButtonTexture.getSize();
        backFromGuideButtonSprite->setPosition({ static_cast<float>(texSize.x) / 2.0f + 40.0f,
                                                 static_cast<float>(window.getSize().y) - static_cast<float>(texSize.y) / 2.0f - 40.0f });
    }



    // HP text
    HPText.setString(to_string(PlayerHP) + "/100");
    HPText.setCharacterSize(40);
    HPText.setFillColor(sf::Color::White);
    HPText.setOutlineColor(sf::Color::Black);
    HPText.setOutlineThickness(4.0f);
    HPText.setOrigin({ HPText.getLocalBounds().size.x / 2.0f, HPText.getLocalBounds().size.y / 2.0f });
    HPText.setPosition({ 2325.0f, 28.0f });

    // Coins text
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

    //Map name text
	MapNameText.setCharacterSize(25);
	MapNameText.setFillColor(sf::Color::White);
	MapNameText.setOutlineColor(sf::Color::Black);
	MapNameText.setOutlineThickness(4.0f);
	MapNameText.setOrigin({ MapNameText.getLocalBounds().size.x / 2.0f, MapNameText.getLocalBounds().size.y / 2.0f });
	MapNameText.setPosition({ (float)window.getSize().x - ((float)statsTableTexture.getSize().x / 2.0f) + 4.0f, 1332.0f });

    MapNameTextBigger.setCharacterSize(40);
    MapNameTextBigger.setFillColor(sf::Color::White);
    MapNameTextBigger.setOutlineColor(sf::Color::Black);
    MapNameTextBigger.setOutlineThickness(4.0f);
    MapNameTextBigger.setOrigin({ MapNameTextBigger.getLocalBounds().size.x / 2.0f, MapNameTextBigger.getLocalBounds().size.y / 2.0f });
    MapNameTextBigger.setPosition({ (float)window.getSize().x / 2.0f, (float)window.getSize().y / 2.0f - 470.0f });

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
    if (mainMenuEndButtonTexture.loadFromFile("assets/ui/button_quit_big.png")) {
        mainMenuEndButtonSprite = std::make_unique<sf::Sprite>(mainMenuEndButtonTexture);
        mainMenuEndButtonSprite->setOrigin({ (float)mainMenuEndButtonTexture.getSize().x / 2.0f, (float)mainMenuEndButtonTexture.getSize().y / 2.0f });
        mainMenuEndButtonSprite->setPosition({ (float)window.getSize().x / 2.0f - 185.0f, (float)window.getSize().y / 2.0f + 585.0f});
    } else {
        std::cerr << "Nie mozna za³adowaæ main_menu.png (assets/ui/button_quit_big.png)" << std::endl;
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

    // --- Artillery explosion animation frames (6 frames) ---
    // Your current files are GIFs: frame_0_delay-0.15s.gif .. frame_5_delay-0.15s.gif
    // If you convert them to PNG, keep the same naming and just change extension here.
    explosionFrameTextures.clear();
    explosionFrames.clear();
    explosionFrameTextures.resize(6);

    for (int i = 0; i < 6; ++i) {
        const std::string path = "assets/bullets/explosion/frame_" + std::to_string(i) + "_delay-0.15s.gif";
        if (!explosionFrameTextures[i].loadFromFile(path)) {
            std::cerr << "Nie mozna za³adowaæ klatki eksplozji: " << path << std::endl;
        } else {
            explosionFrames.push_back(&explosionFrameTextures[i]);
        }
    }

}

void Game::run() {
    std::cerr << "[BOOT] Game::run() entering loop\n";
    sf::Clock clock;
    while (isRunning && window.isOpen()) {
        float dt = clock.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
    std::cerr << "[BOOT] Game::run() exiting loop\n";
}

void Game::processEvents() {
    while (const auto event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            isRunning = false;
            window.close();
        }

        // --- MAIN MENU INPUT ---
        if (gameState == GameState::MAIN_MENU) {
            // Ensure menu music is playing when we are in menu
            if (!g_menuMusicPlaying) {
                soundManager.playBackground("music/main_menu_music.mp3", true, 20.0f);
                g_menuMusicPlaying = true;
            }

            logOnce("[MENU] processEvents() in MAIN_MENU");

            if (auto mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    soundManager.playClickSound();
                    const sf::Vector2f mousePos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
                    const int action = mainMenu.update(mousePos, true);
                    std::cerr << "[MENU] click action=" << action << "\n";

                    if (action == 1) {
                        mainMenu.switchToLevelSelect();
                        std::cerr << "[MENU] switched to LEVEL_SELECT\n";
                    } else if (action == 2) {
                        // Quit game
                        isRunning = false;
                        window.close();
                    } else if (action == 3) {
                        // Back to main menu (from level select)
                        mainMenu.switchToMainMenu();
                    } else if (action >= 100) {
                        const int mapIndex = action - 100;
                        std::cerr << "[MENU] start map index=" << mapIndex << "\n";

                        // Switch away from menu music. loadMapByIndex will start the map music.
                        g_menuMusicPlaying = false;

                        startPlayingMap(mapIndex);
                    }
                }
            }
            continue;
        }

        // --- PAUSE MENU INPUT (must be handled before gameOver/gameplay clicks) ---
        if (inGameMenuActive) {
            if (auto mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    soundManager.playClickSound();
                    const sf::Vector2f mouseWindowPos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
                    std::cerr << "[PAUSE] click at " << mouseWindowPos.x << "," << mouseWindowPos.y << "\n";

                    // If guidebook is active, only guidebook-related buttons work
                    if (guidebookActive) {
                        if (backFromGuideButtonSprite && backFromGuideButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                            guidebookActive = false;
                            continue;
                        }
                        if (changePageButtonSprite && changePageButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                            guidebookPage = (guidebookPage == 1) ? 2 : 1;
							soundManager.playPageFlipSound();
                            if (guidebookSprite) {
                                guidebookSprite->setTexture(guidebookPage == 1 ? guidebook1Texture : guidebook2Texture);
                                const sf::Texture* activeTex = (guidebookPage == 1 ? &guidebook1Texture : &guidebook2Texture);
                                guidebookSprite->setOrigin({ activeTex->getSize().x / 2.0f, activeTex->getSize().y / 2.0f });
                                guidebookSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f });
                            }
                            continue;
                        }
                        continue;
                    }

                    if (resumeButtonSprite) {
                        const bool hit = resumeButtonSprite->getGlobalBounds().contains(mouseWindowPos);
                        std::cerr << "[PAUSE] resume hit=" << hit << "\n";
                        if (hit) {
                            inGameMenuActive = false;
                            continue;
                        }
                    }

                    if (restartButtonSprite) {
                        const bool hit = restartButtonSprite->getGlobalBounds().contains(mouseWindowPos);
                        std::cerr << "[PAUSE] restart hit=" << hit << "\n";
                        if (hit) {
                            inGameMenuActive = false;
                            restartCurrentLevel();
                            continue;
                        }
                    }

                    if (mainMenuButtonSprite) {
                        const bool hit = mainMenuButtonSprite->getGlobalBounds().contains(mouseWindowPos);
                        std::cerr << "[PAUSE] main menu hit=" << hit << "\n";
                        if (hit) {
                            std::cerr << "[MENU] Returning to MAIN_MENU from pause\n";
                            inGameMenuActive = false;
                            guidebookActive = false;
                            victory = false;
                            gameOver = false;

                            towers.clear();
                            enemies.clear();
                            projectiles.clear();
                            levelIcons.clear();
                            closeTowerMenu();
                            closeTowerActionMenu();
                            map.clearPathsAndBuildAreas();

                            // menu music will be ensured on next menu input pass
                            g_menuMusicPlaying = false;

                            // Reset menu state (Menu is non-copyable in SFML 3, so reconstruct it in-place)
                            mainMenu.~Menu();
                            new (&mainMenu) Menu();
                            gameState = GameState::MAIN_MENU;
                            continue;
                        }
                    }

                    if (soundButtonSprite) {
                        const bool hit = soundButtonSprite->getGlobalBounds().contains(mouseWindowPos);
                        std::cerr << "[PAUSE] sound hit=" << hit << "\n";
                        if (hit) {
                            soundOn = !soundOn;
                            soundButtonSprite->setTexture(soundOn ? soundOnTexture : soundOffTexture);
                            continue;
                        }
                    }

                    if (helpButtonSprite) {
                        const bool hit = helpButtonSprite->getGlobalBounds().contains(mouseWindowPos);
                        if (hit) {
                            guidebookActive = true;
                            guidebookPage = 1;
                            if (guidebookSprite) {
                                guidebookSprite->setTexture(guidebook1Texture);
                                guidebookSprite->setOrigin({ guidebook1Texture.getSize().x / 2.0f, guidebook1Texture.getSize().y / 2.0f });
                                guidebookSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f });
                            }
                            continue;
                        }
                    }
                }
            }
            continue;
        }

        // If game over, handle clicks only on overlay buttons
        if (gameOver) {
            if (auto mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    soundManager.playClickSound();
                    sf::Vector2f mouseWindowPos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));
                    if (!victory) {
                        // defeat: click repeat button
                        if (repeatButtonSprite && repeatButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                            // hide end screen immediately
                            gameOver = false;
                            victory = false;
                            restartCurrentLevel();
                        }
                        if (mainMenuEndButtonSprite && mainMenuEndButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                            // Return to main menu (do NOT close the window)
                            std::cerr << "[MENU] Returning to MAIN_MENU from defeat screen\n";
                            gameOver = false;
                            victory = false;
                            towers.clear();
                            enemies.clear();
                            projectiles.clear();
                            levelIcons.clear();
                            closeTowerMenu();
                            closeTowerActionMenu();
                            map.clearPathsAndBuildAreas();
                            // menu music will be ensured on next menu input pass
                            g_menuMusicPlaying = false;
                            mainMenu.~Menu();
                            new (&mainMenu) Menu();
                            gameState = GameState::MAIN_MENU;
						}
                    } else {
                        // victory: click continue button
                        if (continueButtonSprite && continueButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                            gameOver = false;
                            victory = false;
                            goToNextMap();
                        }
                        if (mainMenuEndButtonSprite && mainMenuEndButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                            // Return to main menu (do NOT close the window)
                            std::cerr << "[MENU] Returning to MAIN_MENU from defeat screen\n";
                            gameOver = false;
                            victory = false;
                            towers.clear();
                            enemies.clear();
                            projectiles.clear();
                            levelIcons.clear();
                            closeTowerMenu();
                            closeTowerActionMenu();
                            map.clearPathsAndBuildAreas();
                            // menu music will be ensured on next menu input pass
                            g_menuMusicPlaying = false;
                            mainMenu.~Menu();
                            new (&mainMenu) Menu();
                            gameState = GameState::MAIN_MENU;
                        }
                    }
                }
            }
            continue;
        }

        if (inGameMenuActive) {
            if (auto mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseEvent->button == sf::Mouse::Button::Left) {
                    soundManager.playClickSound();
                    sf::Vector2f mouseWindowPos(static_cast<float>(mouseEvent->position.x), static_cast<float>(mouseEvent->position.y));

                    // If guidebook is active, only guidebook-related buttons work
                    if (guidebookActive) {
                        if (backFromGuideButtonSprite && backFromGuideButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                            guidebookActive = false;
                        }
                        if (changePageButtonSprite && changePageButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                            guidebookPage = (guidebookPage == 1) ? 2 : 1;
                            if (guidebookSprite) {
                                guidebookSprite->setTexture(guidebookPage == 1 ? guidebook1Texture : guidebook2Texture);
                                const sf::Texture* activeTex = (guidebookPage == 1 ? &guidebook1Texture : &guidebook2Texture);
                                guidebookSprite->setOrigin({ activeTex->getSize().x / 2.0f, activeTex->getSize().y / 2.0f });
                                guidebookSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f });
                            }
                        }
                        continue;
                    }

                    if (resumeButtonSprite && resumeButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                        inGameMenuActive = false;
                    }
                    if (mainMenuButtonSprite && mainMenuButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                        // Return to main menu (do NOT close the window)
                        std::cerr << "[MENU] Returning to MAIN_MENU from pause\n";
                        inGameMenuActive = false;
                        gameOver = false;
                        victory = false;

                        towers.clear();
                        enemies.clear();
                        projectiles.clear();
                        levelIcons.clear();
                        closeTowerMenu();
                        closeTowerActionMenu();
                        map.clearPathsAndBuildAreas();

                        // menu music will be ensured on next menu input pass
                        g_menuMusicPlaying = false;

                        mainMenu.~Menu();
                        new (&mainMenu) Menu();
                        gameState = GameState::MAIN_MENU;
                        continue;
                    }
                    if (restartButtonSprite && restartButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                        inGameMenuActive = false;
                        restartCurrentLevel();
                    }
                    if (soundButtonSprite && soundButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
						 soundOn = !soundOn;
						 if (soundOn) {
							 soundButtonSprite->setTexture(soundOnTexture);
						 }
                         else {
                             soundButtonSprite->setTexture(soundOffTexture);
                         }
                     }

                    if (helpButtonSprite && helpButtonSprite->getGlobalBounds().contains(mouseWindowPos)) {
                        guidebookActive = true;
                        guidebookPage = 1;
                        if (guidebookSprite) {
                            guidebookSprite->setTexture(guidebook1Texture);
                            guidebookSprite->setOrigin({ guidebook1Texture.getSize().x / 2.0f, guidebook1Texture.getSize().y / 2.0f });
                            guidebookSprite->setPosition({ static_cast<float>(window.getSize().x) / 2.0f, static_cast<float>(window.getSize().y) / 2.0f });
                        }
                    }
                }
            }
            continue;
        }

        // Klikniêcie myszk¹
        if (auto mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseEvent->button == sf::Mouse::Button::Left) {
                soundManager.playClickSound();
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
						soundManager.playStartWaveSound();
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
            //case sf::Keyboard::Key::Escape:
            //    isRunning = false;
            //    window.close();
            //    break;
            case sf::Keyboard::Key::R:
                showAllRanges = !showAllRanges;
                for (auto& tower : towers) {
                    tower.toggleRangeDisplay(showAllRanges);
                }
                break;
            case sf::Keyboard::Key::Escape:
                inGameMenuActive = !inGameMenuActive;
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

    updateUpgradeCostUI();
}

void Game::closeTowerActionMenu() {
    towerActionMenuActive = false;
    selectedTowerIndex = -1;
    upgradeCostText.setString("");
}

void Game::updateUpgradeCostUI() {
    if (!towerActionMenuActive || selectedTowerIndex < 0 || selectedTowerIndex >= static_cast<int>(towers.size())) {
        upgradeCostText.setString("");
        return;
    }

    if (!upgradeButtonSprite) {
        upgradeCostText.setString("");
        return;
    }

    const Tower& t = towers[selectedTowerIndex];
    if (!t.canUpgrade() || t.getLevel() >= 4) {
        upgradeCostText.setString("MAX");
    } else {
        upgradeCostText.setString(std::to_string(t.getUpgradeCost()));
    }

    // center under the upgrade button
    auto bounds = upgradeCostText.getLocalBounds();
    upgradeCostText.setOrigin({ bounds.position.x + bounds.size.x / 2.0f, bounds.position.y + bounds.size.y / 2.0f });

    const sf::FloatRect btn = upgradeButtonSprite->getGlobalBounds();
    const sf::Vector2f btnCenter(btn.position.x + btn.size.x / 2.0f, btn.position.y + btn.size.y / 2.0f);
    upgradeCostText.setPosition({ btnCenter.x, btnCenter.y + btn.size.y / 2.0f + 18.0f });
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
	soundManager.playTowerUpgradeSound();
    t.upgrade();

    // Update level icon texture for this tower
    if (selectedTowerIndex >= 0 && selectedTowerIndex < static_cast<int>(levelIcons.size()) && levelIcons[selectedTowerIndex]) {
        int lvl = t.getLevel();
        if (lvl == 2) levelIcons[selectedTowerIndex]->setTexture(level2Texture);
        else if (lvl == 3) levelIcons[selectedTowerIndex]->setTexture(level3Texture);
		else if (lvl == 4) levelIcons[selectedTowerIndex]->setTexture(level4Texture);
        else levelIcons[selectedTowerIndex]->setTexture(level1Texture);
    }

    updateUpgradeCostUI();
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
	case EnemyType::BOSS: damage = 50; break;
    }
    PlayerHP -= damage;
    if (PlayerHP < 0) PlayerHP = 0;
    HPText.setString(std::to_string(PlayerHP) + "/100");
}

void Game::update(float deltaTime) {
    if (gameState == GameState::MAIN_MENU) {
        logOnce("[MENU] update() in MAIN_MENU");
        const sf::Vector2i pixelPos = sf::Mouse::getPosition(window);
        const sf::Vector2f mousePos(static_cast<float>(pixelPos.x), static_cast<float>(pixelPos.y));
        (void)mainMenu.update(mousePos, false);
        return;
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

            float explosionRadius = 0.0f;
            float edgeMult = 1.0f;
            if (tower.getType() == TowerType::ARTILLERY) {
                explosionRadius = tower.getExplosionRadius();
                edgeMult = tower.getEdgeDamageMultiplier();
            }

            auto proj = makeProjectileForTower(tower.getType(), tpos, dir, bulletTextures, tower.getDamage(), range, explosionRadius, edgeMult);
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
                // direct hit or proximity hit
                if (p->hasExplosion()) {
                    const float R = p->getExplosionRadius();
                    const float edgeMult = p->getEdgeDamageMultiplier();
                    const sf::Vector2f center = p->getPosition();

                    // Spawn VFX (range indicator) once per impact
                    if (!explosionFrames.empty()) {
                        explosionEffects.emplace_back(center, R, explosionFrames, 0.06f);
                    }

                    // Apply AoE once at impact position: linear falloff from 1.0 at center to edgeMult at radius.
                    for (auto& aoeEnemy : enemies) {
                        if (!aoeEnemy) continue;
                        sf::Vector2f ep = aoeEnemy->getPosition();
                        float ax = ep.x - center.x;
                        float ay = ep.y - center.y;
                        float d2a = ax*ax + ay*ay;
                        if (d2a > R * R) continue;

                        float d = std::sqrt(d2a);
                        float t = (R > 0.0f) ? (d / R) : 1.0f; // 0..1
                        float mult = (1.0f - t) * 1.0f + t * edgeMult;
                        if (mult < 0.0f) mult = 0.0f;

                        int dealt = static_cast<int>(std::round(p->getDamage() * mult));
                        if (dealt > 0) aoeEnemy->takeDamage(dealt);
                    }
                } else {
                    ePtr->takeDamage(p->getDamage());
                }

                it = projectiles.erase(it);
                removed = true;
                break;
            }
        }
        if (!removed) ++it;
    }

    // Update explosion effects
    for (auto it = explosionEffects.begin(); it != explosionEffects.end();) {
        it->update(deltaTime);
        if (!it->isAlive()) it = explosionEffects.erase(it);
        else ++it;
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
    if (gameState == GameState::MAIN_MENU) {
        logOnce("[MENU] render() in MAIN_MENU");
        window.clear(sf::Color::Black);
        mainMenu.draw(window);
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

    // Projectiles and explosion VFX (VFX drawn above enemies, below UI)
    for (const auto& p : projectiles) p->draw(window);
    for (const auto& fx : explosionEffects) fx.draw(window);

    // Draw UI (on top)
    // IMPORTANT: only draw sprites created from valid textures.
    if (hudTexture.getSize().x > 0 && hudTexture.getSize().y > 0) {
        sf::Sprite hudSpriteLocal(hudTexture);
        hudSpriteLocal.setPosition({ (float)window.getSize().x - (float)hudTexture.getSize().x, 0.0f});
        window.draw(hudSpriteLocal);
    }

    sf::Sprite statsTableSpriteLocal(statsTableTexture);
    statsTableSpriteLocal.setOrigin({ (float)statsTableTexture.getSize().x / 2.0f, (float)statsTableTexture.getSize().y / 2.0f });
    statsTableSpriteLocal.setPosition({ (float)window.getSize().x - ((float)statsTableTexture.getSize().x / 2.0f), (float)window.getSize().y - ((float)statsTableTexture.getSize().y / 2.0f)});

    window.draw(statsTableSpriteLocal);
	window.draw(WaveText);
	window.draw(MapNameText);

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

        if (upgradeCostText.getString().getSize() > 0)
            window.draw(upgradeCostText);
    }

    // End screens
    if (gameOver) {
        // Draw dark overlay first, then end screen sprite above
        if ((!victory && defeatSprite) || (victory && victorySprite)) {
            window.draw(screenOverlay);
        }
        if (!victory && defeatSprite) {
            window.draw(*defeatSprite);
            window.draw(MapNameTextBigger);
            window.draw(*mainMenuEndButtonSprite);
            if (repeatButtonSprite)
                window.draw(*repeatButtonSprite);
        } else if (victory && victorySprite) {
            window.draw(*victorySprite);
            window.draw(MapNameTextBigger);
            window.draw(*mainMenuEndButtonSprite);
            if (continueButtonSprite)
                window.draw(*continueButtonSprite);
        }
    }

    if (inGameMenuActive) {
        // In-game menu background setup
        RectangleShape inGameMenuBackground;
        inGameMenuBackground.setFillColor(sf::Color(0, 0, 0, 150)); // pó³przezroczyste czarne t³o
        inGameMenuBackground.setSize(sf::Vector2f((float)window.getSize().x, (float)window.getSize().y));
        inGameMenuBackground.setPosition({ 0.0f, 0.0f });
		window.draw(inGameMenuBackground);

        // If guidebook active, draw it instead of pause panel/buttons
        if (guidebookActive) {
            if (guidebookSprite)
                window.draw(*guidebookSprite);
            if (changePageButtonSprite)
                window.draw(*changePageButtonSprite);
            if (backFromGuideButtonSprite)
                window.draw(*backFromGuideButtonSprite);
        } else {
            if (inGameMenuSprite)
				window.draw(*inGameMenuSprite);
			window.draw(MapNameTextBigger);
            // Draw buttons
            if (resumeButtonSprite)
                window.draw(*resumeButtonSprite);
            if (restartButtonSprite)
                window.draw(*restartButtonSprite);
            if (mainMenuButtonSprite)
                window.draw(*mainMenuButtonSprite);
            if (soundButtonSprite)
				window.draw(*soundButtonSprite);
            if (helpButtonSprite)
                window.draw(*helpButtonSprite);
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

void Game::startPlayingMap(int index) {
    // reset run-time state (same as restarting a level, but selecting a potentially different one)
    towers.clear();
    enemies.clear();
    projectiles.clear();
    levelIcons.clear();
    closeTowerMenu();
    closeTowerActionMenu();
    towerMenuActive = false;
    towerActionMenuActive = false;

    // Clear map paths/build areas to avoid duplicates
    map.clearPathsAndBuildAreas();

    // Reset player HP and HUD
    PlayerHP = 100;
    HPText.setString(std::to_string(PlayerHP) + "/100");
    HPText.setOrigin({ HPText.getLocalBounds().size.x / 2.0f, HPText.getLocalBounds().size.y / 2.0f });

    // Load the chosen map data
    loadMapByIndex(index);

    // restore wave button texture
    if (waveButtonSprite)
        waveButtonSprite->setTexture(buttonWaveStartTexture);

    // Apply menu audio settings
    const GameSettings gs = mainMenu.getSettings();
    soundManager.setBackgroundVolume(gs.volume);

    // switch state
    gameState = GameState::PLAYING;
}

