#pragma once
#include <iostream>
#include <SFML/Audio.hpp>

using namespace sf;
using namespace std;

class SoundManager {
private:
    SoundBuffer notEnoughCoinsBuffer;
    Sound notEnoughCoinsSound;

	SoundBuffer towerPlaceBuffer;
	Sound towerPlaceSound;

public:
    SoundManager();
    void playNotEnoughCoinsSound();
	void playTowerPlaceSound();
};