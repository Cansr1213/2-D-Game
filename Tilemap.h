#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <algorithm>


class Tilemap {
public:
    int tileSize = 32;

    std::vector<std::vector<int>> tiles;

    sf::Texture tilesetTexture;
    sf::Sprite tileSprite;

    Tilemap() = default;


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
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to load level file: " + path);

        }
        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();

            }
            if (!line.empty()) {
                lines.push_back(line);

            }
        }
        if (lines.empty()) {
            return;

        }
        std::size_t maxWidth = 0;
        for (const auto& l : lines) {
            maxWidth = std::max(maxWidth, l.size());


        }
        tiles.assign(lines.size(), std::vector<int>(static_cast<int>(maxWidth), 0));

        for (std::size_t y = 0; y < lines.size(); ++y) {
            for (std::size_t x = 0; x < maxWidth && x < lines[y].size(); ++x) {
                tiles[y][x] = (lines[y][x] == '1') ? 1 : 0;
            }
        }

        

        
    }

    // No update needed yet, but included for engine consistency
    void update(float dt) {
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
