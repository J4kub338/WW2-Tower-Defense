#pragma once
#include "Enemy.hpp"
#include <vector>
#include <queue>
#include <memory>
#include <string>

struct SpawnEntry {
    EnemyType type;
    int count;
    float spawnInterval; // seconds between spawns of this entry
    int pathIndex; // which path to spawn from for this entry
};

struct Wave {
    int waveNumber = 1;
    std::vector<SpawnEntry> entries; // spawn entries with counts and per-entry intervals
};

class WaveManager {
private:
    std::vector<Wave> waves;
    int currentWaveIndex = -1;
    float spawnTimer = 0.0f;
    bool isSpawning = false;

    struct SpawnTask {
        EnemyType type;
        float interval;
        int pathIndex;
    };

    std::queue<SpawnTask> enemiesToSpawn;

public:
    WaveManager();

    void update(float deltaTime,
        std::vector<std::unique_ptr<Enemy>>& enemies,
        const std::vector<sf::Vector2f>& path1,
        const std::vector<sf::Vector2f>& path2,
        const std::string& enemyTexturePrefix);

    void startNextWave();
    void stopCurrentWave();
    bool isWaveComplete() const;

    void addWave(const Wave& wave);

    int getCurrentWave() const { return currentWaveIndex + 1; }
    bool isActive() const { return isSpawning || !enemiesToSpawn.empty(); }
};