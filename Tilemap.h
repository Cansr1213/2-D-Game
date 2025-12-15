#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class Tilemap {
public:
    int tileSize = 64;

    std::vector<std::vector<int>> tiles;

    sf::Texture tilesetTexture;
    sf::Sprite tileSprite;

    Tilemap(int width, int height) {
        tiles.resize(height, std::vector<int>(width, 0));
    }

    // Load tileset texture
    void loadTileset(const std::string& path, int tileW, int tileH) {
        tileSize = tileW;

        if (!tilesetTexture.loadFromFile(path)) {
            throw std::runtime_error("Failed to load tileset");
        }

        tileSprite.setTexture(tilesetTexture);
    }

    // TEMP level generator (safe)
    void loadFromFile(const std::string& path) {
        if (tiles.empty()) return;

        int groundRow = static_cast<int>(tiles.size()) - 1;

        for (int x = 0; x < static_cast<int>(tiles[groundRow].size()); ++x) {
            tiles[groundRow][x] = 1;
        }
    }

    // No update needed yet, but included for engine consistency
    void update(float /*dt*/) {
        // Tilemaps are static for now
    }

    void render(sf::RenderWindow& window) {
        for (int y = 0; y < static_cast<int>(tiles.size()); ++y) {
            for (int x = 0; x < static_cast<int>(tiles[y].size()); ++x) {

                if (tiles[y][x] != 1)
                    continue;

                // Draw ONLY ONE tile region
                tileSprite.setTextureRect(
                    sf::IntRect(0, 0, tileSize, tileSize)
                );

                tileSprite.setPosition(
                    static_cast<float>(x * tileSize),
                    static_cast<float>(y * tileSize)
                );

                window.draw(tileSprite);
            }
        }
    }

    bool isSolid(int x, int y) const {
        if (y < 0 || y >= static_cast<int>(tiles.size())) return false;
        if (x < 0 || x >= static_cast<int>(tiles[y].size())) return false;
        return tiles[y][x] == 1;
    }
};
