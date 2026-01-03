#include "WaveManager.hpp"
#include <iostream>

WaveManager::WaveManager() = default;

void WaveManager::addWave(const Wave& wave) {
    waves.push_back(wave);
}

bool WaveManager::isWaveComplete() const {
    return !isSpawning && enemiesToSpawn.empty();
}

void WaveManager::startNextWave() {
    if (currentWaveIndex + 1 >= static_cast<int>(waves.size())) {
        std::cout << "No more waves to start\n";
        return;
    }
    currentWaveIndex++;
    const Wave& w = waves[currentWaveIndex];

    // Fill queue with spawn tasks according to entries
    while (!enemiesToSpawn.empty()) enemiesToSpawn.pop();
    for (const auto& entry : w.entries) {
        for (int i = 0; i < entry.count; ++i) {
            enemiesToSpawn.push({ entry.type, entry.spawnInterval, entry.pathIndex });
        }
    }

    isSpawning = true;
    spawnTimer = 0.0f;
    std::cout << "Starting wave " << w.waveNumber << "\n";
}

void WaveManager::stopCurrentWave() {
    // Pauses spawning but keeps queued enemies; toggle isSpawning off
    isSpawning = false;
    std::cout << "Wave " << (currentWaveIndex + 1) << " paused\n";
}

void WaveManager::update(float deltaTime,
    std::vector<std::unique_ptr<Enemy>>& enemies,
    const std::vector<sf::Vector2f>& path1,
    const std::vector<sf::Vector2f>& path2) {

    if (!isSpawning) return;

    if (enemiesToSpawn.empty()) {
        isSpawning = false;
        return;
    }

    spawnTimer += deltaTime;

    // Peek the current task to know its interval
    while (!enemiesToSpawn.empty()) {
        const SpawnTask& task = enemiesToSpawn.front();
        if (spawnTimer < task.interval) break;

        // spawn it
        const std::vector<sf::Vector2f>& chosenPath = (task.pathIndex == 0) ? path1 : path2;
        enemies.emplace_back(std::make_unique<Enemy>(task.type, chosenPath));

        spawnTimer -= task.interval;
        enemiesToSpawn.pop();
    }
}
