#pragma once
#include <SFML/Graphics.hpp>
#include "Map.hpp"
#include "Tower.hpp"
#include "WaveManager.hpp"
#include <vector>
#include <unordered_map>
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Font.hpp"
#include "Projectile.hpp"
#include "Sounds.hpp"


using namespace sf;
using namespace std;   

struct MapInfo {
    std::string mapFile; // path to map file
    std::string enemyTexturePrefix; // optional override for enemy texture base names
    std::vector<Wave> waves; // waves specific to this map
    int playerCoins = 300; // initial player coins for this map
    int numberOfWaves = waves.size();
	int Soundtracknumber = 1; // soundtrack index
};

class Game {
private:
    sf::RenderWindow window;
    Map map;
    std::vector<Tower> towers;
    std::vector<std::unique_ptr<Enemy>> enemies;
    WaveManager waveManager;
    TowerType selectedTowerType;
    bool showAllRanges;
    bool isRunning;
    std::unordered_map<TowerType, sf::Texture> towerTextures;
    std::unordered_map<TowerType, sf::Texture> bulletTextures;
    int PlayerHP = 100;
    sf::Font font;
    Font fontForText;
    Text HPText;
    Text CoinsText;
    Text WaveText;
    Text KillsText;
    int PlayerCoins = 300;


	//hud textures and sprites
    Texture hudTexture;
    Texture statsTableTexture;
    std::unique_ptr<Sprite> hudSprite; // SFML 3 Sprite has no default ctor

    // level indicator texture/sprite (global icons)
    sf::Texture level1Texture;
    sf::Texture level2Texture;
    sf::Texture level3Texture;
	Texture level4Texture;
    std::vector<std::unique_ptr<sf::Sprite>> levelIcons; // one per tower index

    std::vector<std::unique_ptr<Projectile>> projectiles;


    //Sounds
    SoundManager soundManager;

    // Single wave control button (textured)
    sf::Texture buttonWaveStartTexture;
    sf::Texture buttonWaveRunningTexture;
    std::unique_ptr<sf::Sprite> waveButtonSprite;

    // Low-coins message
    Text lowCoinsText;
    bool showLowCoinsMessage = false;
    float lowCoinsMessageTimer = 0.0f; // seconds

    Text MaxLevelText;
	bool showMaxLevelMessage = false;
	float maxLevelMessageTimer = 0.0f; // seconds

    // Map selection
    std::vector<MapInfo> availableMaps;
    int selectedMapIndex = 1;
    std::string currentMapEnemyPrefix; // prefix like "GER" or "UK"

    // Radial tower selection menu
    bool towerMenuActive = false;
    sf::Vector2f towerMenuCenter;
    sf::Vector2f pendingBuildCenter; // exact center of claimed build area
    struct TowerOption { TowerType type; sf::Sprite sprite; };
    std::vector<TowerOption> towerMenuOptions;
    std::unordered_map<TowerType, sf::Texture> towerIconTextures; // assets/icons

    // Tower action menu (upgrade/destroy)
    bool towerActionMenuActive = false;
    sf::Vector2f towerActionCenter;
    sf::Texture upgradeButtonTexture;
    sf::Texture destroyButtonTexture;
    std::unique_ptr<sf::Sprite> upgradeButtonSprite;
    std::unique_ptr<sf::Sprite> destroyButtonSprite;
    int selectedTowerIndex = -1; // index in towers vector


    // Wave UI state
    int currentWaveNumber = 0;
    int numberOfWaves = 0;
    int KillsCount = 0;

    // Victory/Defeat state
    bool gameOver = false;
    bool victory = false;
    sf::Texture defeatTexture;
    sf::Texture victoryTexture;
    std::unique_ptr<sf::Sprite> defeatSprite;
    std::unique_ptr<sf::Sprite> victorySprite;
    sf::RectangleShape screenOverlay; // dark overlay (blur substitute)
    // End screen buttons
    sf::Texture repeatButtonTexture;   // defeat screen
    sf::Texture continueButtonTexture; // victory screen
    std::unique_ptr<sf::Sprite> repeatButtonSprite;
    std::unique_ptr<sf::Sprite> continueButtonSprite;

    // Metody prywatne
    void processEvents();
    void update(float deltaTime);
    void render();
    void handleMouseClick(const sf::Vector2f& position);
    void HPTextUpdate();

    void loadMapByIndex(int index);

    void openTowerMenu(const sf::Vector2f& center);
    void closeTowerMenu();

    // tower actions
    void openTowerActionMenu(int towerIndex);
    void closeTowerActionMenu();
    void tryUpgradeSelectedTower();
    void destroySelectedTower();

    // end game helpers
    void triggerDefeat();
    void triggerVictory();
    void restartCurrentLevel();
    void goToNextMap();

public:
    Game();
    void run();
};