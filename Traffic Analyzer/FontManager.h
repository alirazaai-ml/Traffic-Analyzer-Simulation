#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class FontManager {
private:
    sf::Font font;
    static FontManager* instance;

    FontManager(); // Private constructor

public:
    static FontManager& getInstance();
    sf::Font& getFont() { return font; }
    bool isFontLoaded() const { return fontLoaded; }

private:
    bool fontLoaded;
};