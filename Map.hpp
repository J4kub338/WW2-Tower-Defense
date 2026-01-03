#pragma once
#include <SFML/Graphics.hpp> 
#include <iostream>
#include <vector>
#include <string>
#include <memory>

using namespace sf;
using namespace std;

class Map {
private:
    Texture bgtexture;
    vector<Vector2f> pathpoints;
    vector<Vector2f> pathpoints2;
    struct BuildArea { RectangleShape shape; bool occupied = false; };
    vector<BuildArea> buildAreas; // Obszary budowania
    vector<CircleShape> pathVisual;    // Wizualizacja œcie¿ki

    
public:
    Map();                              // deklaracja konstruktora (tylko raz)

    //dodawanie œcie¿ek
    void addPathPoint(float x, float y);
    void addPathPoint2(float x, float y);

    void addPathPoint(const Vector2f& point);
    void addPathPoint2(const Vector2f& point);

    //dodawanie obszarów budowania (wie¿)
    void addBuildArea(float x, float y, float width, float height);

    bool canBuildHere(const Vector2f& position) const;
    // Próbuje zarezerwowaæ obszar budowy. Jeœli dostêpny, zwraca true i ustawia outCenter na œrodek obszaru
    bool tryClaimBuildArea(const Vector2f& position, Vector2f& outCenter);

    //rysowanie
    void draw(RenderWindow& window) const;

    //gettery
    const std::vector<sf::Vector2f>& getPathPoints() const { return pathpoints; }
    const sf::Vector2f& getPathStart() const { return pathpoints.front(); }
    const sf::Vector2f& getPathEnd() const { return pathpoints.back(); }
    const std::vector<sf::Vector2f>& getPathPoints2() const { return pathpoints2; }
    const sf::Vector2f& getPath2Start() const { return pathpoints2.front(); }
    const sf::Vector2f& getPath2End() const { return pathpoints2.back(); }
    const Texture& getBackgroundTexture() const { return bgtexture; }

    //³adowanie danych mapy z pliku
    bool loadFromFile(const std::string& filename);
};