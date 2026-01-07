#include "Sounds.hpp"

SoundManager::SoundManager()
	: MouseClickSound(MouseClickBuffer),
	notEnoughCoinsSound(notEnoughCoinsBuffer),
	towerPlaceSound(towerPlaceBuffer),
	IPshootSound(IPshootBuffer),
	MGshootSound(MGshootBuffer),
	ARTshootSound(ARTshootBuffer),
	AAshootSound(AAshootBuffer)
{
	if (!MouseClickBuffer.loadFromFile("sounds/mouse_click.ogg")) {
		cerr << "Nie mozna za³adowaæ dŸwiêku: assets/sounds/mouse_click.wav" << '\n';
	}
	if (!notEnoughCoinsBuffer.loadFromFile("sounds/brak_monet.ogg")) {
		cerr << "Nie mozna za³adowaæ dŸwiêku: assets/sounds/brak_monet.ogg" << '\n';
	}
	if (!towerPlaceBuffer.loadFromFile("sounds/building_sound.mp3")) {
		cerr << "Nie mozna za³adowaæ dŸwiêku: assets/sounds/tower_place.wav" << '\n';
	}
	if (!IPshootBuffer.loadFromFile("sounds/shooting/IP.mp3")) {
		cerr << "Nie mozna za³adowaæ dŸwiêku: assets/sounds/IP_shoot.wav" << '\n';
	}
	if (!MGshootBuffer.loadFromFile("sounds/shooting/MG.mp3")) {
		cerr << "Nie mozna za³adowaæ dŸwiêku: assets/sounds/MG_shoot.wav" << '\n';
	}
	if (!ARTshootBuffer.loadFromFile("sounds/shooting/ART.mp3")) {
		cerr << "Nie mozna za³adowaæ dŸwiêku: assets/sounds/ART_shoot.wav" << '\n';
	}
	if (!AAshootBuffer.loadFromFile("sounds/shooting/AA.mp3")) {
		cerr << "Nie mozna za³adowaæ dŸwiêku: assets/sounds/AA_shoot.wav" << '\n';
	}
	MouseClickSound.setVolume(15.0f);
	AAshootSound.setVolume(50.0f);

	// Try to open default background music (optional)
	// backgroundMusic.openFromFile("sounds/background.ogg");
	// backgroundMusic.setLoop(true);
	// backgroundMusic.setVolume(60.0f);
}
void SoundManager::playNotEnoughCoinsSound() {
	notEnoughCoinsSound.play();
}
void SoundManager::playTowerPlaceSound() {
	towerPlaceSound.play();
}
void SoundManager::playIPshootSound() {
	IPshootSound.play();
}
void SoundManager::playMGshootSound() {
	MGshootSound.play();
}
void SoundManager::playARTshootSound() {
	ARTshootSound.play();
}
void SoundManager::playAAshootSound() {
	AAshootSound.play();
}
void SoundManager::playClickSound() {
	MouseClickSound.play();
}

bool SoundManager::playBackground(const std::string& file, bool loop, float volume) {
	if (!backgroundMusic.openFromFile(file)) {
		cerr << "Nie mozna za³adowaæ muzyki: " << file << '\n';
		return false;
	}
	backgroundMusic.setLooping(loop);
	backgroundMusic.setVolume(volume);
	backgroundMusic.play();
	return true;
}

void SoundManager::stopBackground() {
	backgroundMusic.stop();
}

void SoundManager::setBackgroundVolume(float volume) {
	backgroundMusic.setVolume(volume);
}