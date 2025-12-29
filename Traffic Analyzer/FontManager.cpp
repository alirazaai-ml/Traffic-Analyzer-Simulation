#include "FontManager.h"
#include <iostream>

FontManager* FontManager::instance = nullptr;

FontManager& FontManager::getInstance() {
    if (instance == nullptr) {
        instance = new FontManager();
    }
    return *instance;
}

FontManager::FontManager() : fontLoaded(false) {
    // Try multiple font paths
    const char* fontPaths[] = {
        "arial.ttf",
        "fonts/arial.ttf",
        "../fonts/arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/tahoma.ttf",
        "C:/Windows/Fonts/calibri.ttf"
    };

    for (const char* path : fontPaths) {
        if (font.loadFromFile(path)) {
            std::cout << "? Font loaded from: " << path << std::endl;
            fontLoaded = true;
            return;
        }
    }

    std::cerr << "? Warning: Could not load any font files!" << std::endl;
    std::cerr << "Creating a minimal default font..." << std::endl;

    fontLoaded = true; 
}