#include "Sounds.hpp"

SoundManager::SoundManager()
	: notEnoughCoinsSound(notEnoughCoinsBuffer),
	towerPlaceSound(towerPlaceBuffer)
{
	if (!notEnoughCoinsBuffer.loadFromFile("sounds/brak_monet.ogg")) {
		cerr << "Nie mozna za³adowaæ dŸwiêku: assets/sounds/not_enough_coins.wav" << '\n';
	}
	if (!towerPlaceBuffer.loadFromFile("sounds/building_sound.mp3")) {
		cerr << "Nie mozna za³adowaæ dŸwiêku: assets/sounds/tower_place.wav" << '\n';
	}

}
void SoundManager::playNotEnoughCoinsSound() {
	notEnoughCoinsSound.play();
}
void SoundManager::playTowerPlaceSound() {
	towerPlaceSound.play();
}