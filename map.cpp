#include "Map.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>

#define MAP_DEBUG //okdomentuj by w³¹czyæ debugowanie


Map::Map() = default; // unique_ptr domyœlnie nullptr

// Œcie¿ka 1
void Map::addPathPoint(float x, float y) {
	pathpoints.emplace_back(x, y);

	// WIZUALIZACJA (wy³¹czyæ przy normalnej grze)
	sf::CircleShape point;
	point.setRadius(5.0f);
	point.setOrigin({ 5.0f, 5.0f }); // œrodek kó³ka w (x,y)
	point.setFillColor(sf::Color::Red);
	point.setPosition({ x, y });
	pathVisual.push_back(point);
}

void Map::addPathPoint(const sf::Vector2f& point) {
	addPathPoint(point.x, point.y);
}

// Œcie¿ka 2
void Map::addPathPoint2(float x, float y) {
	pathpoints2.emplace_back(x, y);

	// WIZUALIZACJA (wy³¹czyæ przy normalnej grze)
	sf::CircleShape point;
	point.setRadius(5.0f);
	point.setOrigin({ 5.0f, 5.0f });
	point.setFillColor(sf::Color::Red);
	point.setPosition({ x, y });
	pathVisual.push_back(point);
}

void Map::addPathPoint2(const sf::Vector2f& point) {
	addPathPoint2(point.x, point.y);
}

// Dodawanie obszarów budowania (wie¿)
// Uwaga: tutaj (x,y) traktujemy jako œrodek prostok¹ta (zgodne z Twoim widokiem).
void Map::addBuildArea(float x, float y, float width, float height) {
	RectangleShape area;
	area.setSize({ width, height });
	area.setOrigin({ width / 2.0f - 32.0f, height / 2.0f - 32.0f}); // center origin
	area.setPosition({ x, y });

	// WIZUALIZACJA
	area.setFillColor(sf::Color(0, 255, 0, 0));
	area.setOutlineThickness(2.0f);
	area.setOutlineColor(sf::Color::Black);
	buildAreas.push_back({ area, false });
}

bool Map::canBuildHere(const sf::Vector2f& position) const {
	for (size_t i = 0; i < buildAreas.size(); ++i) {
		const auto& area = buildAreas[i].shape;
		sf::FloatRect bounds = area.getGlobalBounds();

#ifdef MAP_DEBUG
		std::cout << "BuildArea[" << i << "] bounds = ("
			<< bounds.position.x << ", " << bounds.position.y << ", "
			<< bounds.size.x << ", " << bounds.size.y << ")\n";
		std::cout << "Click at (" << position.x << ", " << position.y << ")\n";
#endif

		if (bounds.contains({ position.x, position.y })) {

			return !buildAreas[i].occupied;
		}
	}
	return false;
}

// Próbuje zarezerwowaæ obszar budowy. Jeœli dostêpny, zwraca true i ustawia outCenter na œrodek obszaru
bool Map::tryClaimBuildArea(const sf::Vector2f& position, sf::Vector2f& outCenter) {
	for (size_t i = 0; i < buildAreas.size(); ++i) {
		auto& b = buildAreas[i];
		sf::FloatRect bounds = b.shape.getGlobalBounds();
		if (bounds.contains(position)) {
			if (b.occupied) return false;
			b.occupied = true;
			// center = position of shape (since origin is set to center in addBuildArea)
			outCenter = b.shape.getPosition();
			return true;
		}
	}
	return false;
}

// Rysowanie
void Map::draw(sf::RenderWindow& window) const {
	for (const auto& ba : buildAreas) {
		window.draw(ba.shape);
	}
	for (const auto& point : pathVisual) {
		window.draw(point);
	}
}

// £adowanie danych mapy z pliku
bool Map::loadFromFile(const std::string& filename) {
#ifdef MAP_DEBUG
	int i = 1;
#endif
	std::ifstream file(filename);
	if (!file.is_open()) {
		std::cerr << "Nie otwarto pliku mapy: " << filename << std::endl;
		return false;
	}

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#') continue;

		std::stringstream ss(line);
		std::string type;
		ss >> type;

		if (type == "BACKGROUND") {
			std::string bgFile;
			ss >> bgFile;
			if (!bgtexture.loadFromFile(bgFile)) {
				cerr << "Nie mozna zaladowac tekstury tla: " << bgFile << std::endl;
			}
#ifdef MAP_DEBUG
			cout << "Zaladowano tlo z pliku: " << bgFile << endl;
#endif

		}
		else if (type == "PATH") {
			float x, y;
			ss >> x >> y;
			addPathPoint(x, y);
#ifdef MAP_DEBUG
			cout << "Dodano punkt sciezki 1: (" << x << ", " << y << ")" << endl;
#endif
		}
		else if (type == "PATH2") {
			float x, y;
			ss >> x >> y;
			addPathPoint2(x, y);
#ifdef MAP_DEBUG
			cout << "Dodano punkt sciezki 2: (" << x << ", " << y << ")" << endl;
#endif
		}
		else if (type == "BUILD_AREA") {
			float x, y, w, h;
			ss >> x >> y >> w >> h;
			addBuildArea(x, y, w, h);
#ifdef MAP_DEBUG
			cout << "Dodano obszar budowy " << i << ": (" << x << ", " << y << ", " << w << ", " << h << ")" << endl;
			i++;
#endif
		}
	} // <-- BRAKOWA£O TEGO ZAMKNIÊCIA PÊTLI while

	file.close();
	return true;
}