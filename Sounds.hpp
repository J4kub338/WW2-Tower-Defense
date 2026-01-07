#pragma once
#include <iostream>
#include <SFML/Audio.hpp>

using namespace sf;
using namespace std;

class SoundManager {
private:
	SoundBuffer MouseClickBuffer;
	Sound MouseClickSound;

    SoundBuffer notEnoughCoinsBuffer;
    Sound notEnoughCoinsSound;

	SoundBuffer towerPlaceBuffer;
	Sound towerPlaceSound;

	SoundBuffer IPshootBuffer;
	Sound IPshootSound;

	SoundBuffer MGshootBuffer;
	Sound MGshootSound;

	SoundBuffer ARTshootBuffer;
	Sound ARTshootSound;

	SoundBuffer AAshootBuffer;
	Sound AAshootSound;

    // Background music
    sf::Music backgroundMusic;

public:
    SoundManager();
    void playNotEnoughCoinsSound();
	void playTowerPlaceSound();
	void playIPshootSound();
	void playMGshootSound();
	void playARTshootSound();
	void playAAshootSound();
	void playClickSound();

    // Music control
    bool playBackground(const std::string& file, bool loop = true, float volume = 60.0f);
    void stopBackground();
    void setBackgroundVolume(float volume);
};