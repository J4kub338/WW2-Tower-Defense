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
#include "ExplosionEffect.hpp"
#include "Menu.hpp"


using namespace sf;
using namespace std;

struct MapInfo {
    std::string mapFile; // path to map file
    std::string enemyTexturePrefix; // optional override for enemy texture base names
    std::vector<Wave> waves; // waves specific to this map
    int playerCoins = 300; // initial player coins for this map
    int numberOfWaves = waves.size();
    int Soundtracknumber = 1; // soundtrack index
    string mapName = "Default Map"; // map display name
};

class Game {
private:
    enum class GameState {
        MAIN_MENU,
        PLAYING
    };

    GameState gameState = GameState::MAIN_MENU;
    Menu mainMenu;

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
    Text MapNameText;
    Text MapNameTextBigger;
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

    // Artillery explosion VFX (6-frame animation)
    std::vector<sf::Texture> explosionFrameTextures;
    std::vector<const sf::Texture*> explosionFrames; // pointers into explosionFrameTextures
    std::vector<ExplosionEffect> explosionEffects;


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
    int selectedMapIndex = 4;
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

    // UI: upgrade cost label (shown under upgrade button when action menu is open)
    sf::Text upgradeCostText;


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
	Texture mainMenuEndButtonTexture; // both screens
    std::unique_ptr<sf::Sprite> repeatButtonSprite;
    std::unique_ptr<sf::Sprite> continueButtonSprite;
	unique_ptr <Sprite> mainMenuEndButtonSprite;


    //In game menu

    bool inGameMenuActive = false;
    bool soundOn = true;
    RectangleShape inGameMenuBackground;
    unique_ptr <Sprite> inGameMenuSprite;
    unique_ptr <Sprite> restartButtonSprite;
    unique_ptr <Sprite> mainMenuButtonSprite;
    unique_ptr <Sprite> soundButtonSprite;
    unique_ptr <Sprite> resumeButtonSprite;
    Texture resumeButtonTexture;
    Texture mainMenuButtonTexture;
    Texture restartButtonTexture;
    Texture inGameMenuTexture;
    Texture soundOnTexture;
    Texture soundOffTexture;

    // --- Guidebook / Help (pause menu) ---
    bool guidebookActive = false;
    int guidebookPage = 1; // 1 or 2

    sf::Texture helpButtonTexture;
    std::unique_ptr<sf::Sprite> helpButtonSprite;

    sf::Texture guidebook1Texture;
    sf::Texture guidebook2Texture;
    std::unique_ptr<sf::Sprite> guidebookSprite;

    sf::Texture changePageButtonTexture;
    std::unique_ptr<sf::Sprite> changePageButtonSprite;

    sf::Texture backFromGuideButtonTexture;
    std::unique_ptr<sf::Sprite> backFromGuideButtonSprite;


    // Metody prywatne
    void processEvents();
    void update(float deltaTime);
    void render();
    void handleMouseClick(const sf::Vector2f& position);
    void HPTextUpdate();

    void loadMapByIndex(int index);
    void startPlayingMap(int index);

    void openTowerMenu(const sf::Vector2f& center);
    void closeTowerMenu();

    // tower actions
    void openTowerActionMenu(int towerIndex);
    void closeTowerActionMenu();
    void tryUpgradeSelectedTower();
    void destroySelectedTower();
    void updateUpgradeCostUI();

    // end game helpers
    void triggerDefeat();
    void triggerVictory();
    void restartCurrentLevel();
    void goToNextMap();

public:
    Game();
    void run();
};