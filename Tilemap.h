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
    std::vector<sf::Vector2i> spawnTiles;
    std::vector<sf::Vector2i> goalTiles;
    std::vector<sf::Vector2f> collectibles;
    std::vector<bool> collectibleCollected;


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
        spawnTiles.clear();
        goalTiles.clear();
        collectibles.clear();
        collectibleCollected.clear();



        for (std::size_t y = 0; y < lines.size(); ++y) {
            for (std::size_t x = 0; x < maxWidth && x < lines[y].size(); ++x) {

                char c = lines[y][x];
                int tileVal = 0;

                switch (c) {
                case '1':
                    tileVal = 1; // Solid platform
                    break;
                case 'S':
                case 's':
                    spawnTiles.push_back(sf::Vector2i(static_cast<int>(x), static_cast<int>(y)));
                    break;
                case 'G':
                case 'g':
                    goalTiles.push_back(sf::Vector2i(static_cast<int>(x), static_cast<int>(y)));
                    break;
                case 'C':
                case 'c': {
                    const float worldX = static_cast<float>(x * tileSize + tileSize / 2);
                    const float worldY = static_cast<float>(y * tileSize + tileSize / 2);
                    collectibles.emplace_back(worldX, worldY);
                    collectibleCollected.push_back(false);
                    break;
                }
                default:
                    break;
                }

                tiles[y][x] = tileVal;
                
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
        sf::CircleShape coinShape(static_cast<float>(tileSize) * 0.35f);
        coinShape.setFillColor(sf::Color(255, 215, 0));
        coinShape.setOrigin(coinShape.getRadius(), coinShape.getRadius());

        for (std::size_t i = 0; i < collectibles.size(); ++i) {
            if (i < collectibleCollected.size() && collectibleCollected[i])
                continue;

            coinShape.setPosition(collectibles[i]);
            window.draw(coinShape);
        }

        // Render goals as highlighted tiles
        sf::RectangleShape goalShape(sf::Vector2f(static_cast<float>(tileSize), static_cast<float>(tileSize)));
        goalShape.setFillColor(sf::Color(100, 200, 255, 180));
        for (const auto& tile : goalTiles) {
            goalShape.setPosition(static_cast<float>(tile.x * tileSize), static_cast<float>(tile.y * tileSize));
            window.draw(goalShape);
        }


    }

    bool isSolid(int x, int y) const {
        if (y < 0 || y >= static_cast<int>(tiles.size())) return false;
        if (x < 0 || x >= static_cast<int>(tiles[y].size())) return false;
        return tiles[y][x] == 1;
    }

    int getWidth() const {
        return tiles.empty() ? 0 : static_cast<int>(tiles[0].size());
    }

    int getHeight() const {
        return static_cast<int>(tiles.size());
    }

    int getPixelWidth() const {
        return getWidth() * tileSize;
    }

    int getPixelHeight() const {
        return getHeight() * tileSize;
    }

    bool hasSpawnPoint() const {
        return !spawnTiles.empty();
    }

    sf::Vector2f getSpawnPoint() const {
        if (!hasSpawnPoint())
            return sf::Vector2f(0.f, 0.f);

        const sf::Vector2i tile = spawnTiles.front();
        return sf::Vector2f(static_cast<float>(tile.x * tileSize), static_cast<float>(tile.y * tileSize));
    }

    int getCollectibleCount() const {
        return static_cast<int>(collectibles.size());
    }

    int getCollectedCount() const {
        int count = 0;
        for (bool taken : collectibleCollected) {
            if (taken) ++count;
        }
        return count;
    }

    bool collectIfOverlapping(const sf::FloatRect& bounds) {
        bool collectedAny = false;
        for (std::size_t i = 0; i < collectibles.size() && i < collectibleCollected.size(); ++i) {
            if (collectibleCollected[i])
                continue;

            if (bounds.contains(collectibles[i])) {
                collectibleCollected[i] = true;
                collectedAny = true;
            }
        }
        return collectedAny;
    }

    bool reachedGoal(const sf::FloatRect& bounds) const {
        for (const auto& tile : goalTiles) {
            const sf::FloatRect goalRect(
                static_cast<float>(tile.x * tileSize),
                static_cast<float>(tile.y * tileSize),
                static_cast<float>(tileSize),
                static_cast<float>(tileSize));

            if (goalRect.intersects(bounds))
                return true;
        }
        return false;
    }
};
