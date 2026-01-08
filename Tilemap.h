#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <iostream>



class Tilemap {
public:
    int tileSize = 32;
    int tileSourceWidth = 32;
    int tileSourceHeight = 32;
    int tilesetColumns = 1;
    int tilesetRows = 1;
    float tileScaleX = 1.f;
    float tileScaleY = 1.f;

    std::vector<std::vector<int>> tiles;
    std::vector<sf::Vector2i> spawnTiles;
    std::vector<sf::Vector2i> enemySpawnTiles;
    std::vector<sf::Vector2i> goalTiles;
    std::vector<sf::Vector2f> collectibles;
    std::vector<bool> collectibleCollected;
    std::vector<sf::Vector2f> powerups;
    std::vector<bool> powerupCollected;


    sf::Texture tilesetTexture;
    sf::Sprite tileSprite;
    sf::Texture powerupTexture;
    sf::Sprite powerupSprite;
    bool powerupTextureLoaded = false;
    float powerupAnimTime = 0.f;
    float powerupBobAmplitude = 5.f;
    float powerupBobSpeed = 2.2f;
    float powerupPulseSpeed = 3.1f;
    float powerupBaseScaleX = 1.f;
    float powerupBaseScaleY = 1.f;
    bool warnedInvalidTileIndex = false;

    Tilemap() = default;


    Tilemap(int width, int height) {
        tiles.resize(height, std::vector<int>(width, 0));
    }

    // Load tileset texture
    void loadTileset(const std::string& path, int tileW, int tileH) {
        tileSourceWidth = tileW;
        tileSourceHeight = tileH;
        if (tileSourceWidth <= 0 || tileSourceHeight <= 0) {
            throw std::runtime_error("Tileset tile size must be posistive");
        }


        if (!tilesetTexture.loadFromFile(path)) {
            throw std::runtime_error("Failed to load tileset");
        }

        const auto textureSize = tilesetTexture.getSize();
        const unsigned int remainderX = textureSize.x % static_cast<unsigned int>(tileSourceWidth);
        const unsigned int remainderY = textureSize.y % static_cast<unsigned int>(tileSourceHeight);
        if (remainderX != 0 || remainderY != 0) {
            std::cerr << "Warning: tileset size " << textureSize.x << "x" << textureSize.y
                << " is not divisible by tile size " << tileSourceWidth << "x" << tileSourceHeight << ".\n";
        }

        tileSprite.setTexture(tilesetTexture);
        tilesetColumns = std::max(1, static_cast<int>(tilesetTexture.getSize().x) / tileSourceWidth);
        tilesetRows = std::max(1, static_cast<int>(tilesetTexture.getSize().y) / tileSourceHeight);
        tileScaleX = static_cast<float>(tileSize) / static_cast<float>(tileSourceWidth);
        tileScaleY = static_cast<float>(tileSize) / static_cast<float>(tileSourceHeight);
        tileSprite.setScale(tileScaleX, tileScaleY);
        std::cout << "Loaded tileset " << path << " (" << textureSize.x << "x" << textureSize.y
            << "), tile source " << tileSourceWidth << "x" << tileSourceHeight
            << ", grid " << tilesetColumns << "x" << tilesetRows << ".\n";
        powerupTextureLoaded = powerupTexture.loadFromFile("Assets/powerup.png");
        if (powerupTextureLoaded) {
            powerupSprite.setTexture(powerupTexture);
            powerupSprite.setOrigin(
                static_cast<float>(powerupTexture.getSize().x) / 2.f,
                static_cast<float>(powerupTexture.getSize().y) / 2.f);
            const float targetSize = static_cast<float>(tileSize) * 1.5f;
            const float scaleX = targetSize / static_cast<float>(powerupTexture.getSize().x);
            const float scaleY = targetSize / static_cast<float>(powerupTexture.getSize().y);

            powerupSprite.setScale(scaleX, scaleY);
            powerupBaseScaleX = scaleX;
            powerupBaseScaleY = scaleY;

        }
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
        enemySpawnTiles.clear();
        goalTiles.clear();
        collectibles.clear();
        collectibleCollected.clear();
        powerups.clear();
        powerupCollected.clear();




        for (std::size_t y = 0; y < lines.size(); ++y) {
            for (std::size_t x = 0; x < maxWidth && x < lines[y].size(); ++x) {

                char c = lines[y][x];
                int tileVal = 0;
                
             
                switch (c) {
                case 'S':
                case 's':
                    spawnTiles.push_back(sf::Vector2i(static_cast<int>(x), static_cast<int>(y)));
                    break;
                case 'E':
                case 'e':
                    enemySpawnTiles.push_back(sf::Vector2i(static_cast<int>(x), static_cast<int>(y)));
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
                case 'M':
                case 'm': {
                    const float worldX = static_cast<float>(x * tileSize + tileSize / 2);
                    const float worldY = static_cast<float>(y * tileSize + tileSize / 2);
                    powerups.emplace_back(worldX, worldY);
                    powerupCollected.push_back(false);
                    break;
                }
                default:
                    tileVal = tileIndexFromChar(c);
                    break;
                }

                tiles[y][x] = tileVal;
                
            }
        }

        

        
    }

    // No update needed yet, but included for engine consistency
    void update(float dt) {
        powerupAnimTime += dt;

        
    }

    void render(sf::RenderTarget& target) {
        for (int y = 0; y < static_cast<int>(tiles.size()); ++y) {
            for (int x = 0; x < static_cast<int>(tiles[y].size()); ++x) {

                if (tiles[y][x] <= 0)
                    continue;

                // Draw ONLY ONE tile region
                const int maxIndex = tilesetColumns * tilesetRows - 1;
                const int rawIndex = tiles[y][x] - 1;
                if (!warnedInvalidTileIndex && rawIndex > maxIndex) {
                    warnedInvalidTileIndex = true;
                    std::cerr << "Warning: level references tile index " << tiles[y][x]
                        << " but tileset grid supports up to " << (maxIndex + 1)
                        << " tiles.\n";
                }
                const int safeIndex = std::clamp(rawIndex, 0, maxIndex);
                const int tileX = (safeIndex % tilesetColumns) * tileSourceWidth;
                const int tileY = (safeIndex / tilesetColumns) * tileSourceHeight;
                tileSprite.setTextureRect(
                    sf::IntRect(tileX, tileY, tileSourceWidth, tileSourceHeight)
                );

                tileSprite.setPosition(
                    static_cast<float>(x * tileSize),
                    static_cast<float>(y * tileSize)
                );

                target.draw(tileSprite);
            }
        }
        sf::CircleShape coinShape(static_cast<float>(tileSize) * 0.35f);
        coinShape.setFillColor(sf::Color(255, 215, 0));
        coinShape.setOrigin(coinShape.getRadius(), coinShape.getRadius());

        for (std::size_t i = 0; i < collectibles.size(); ++i) {
            if (i < collectibleCollected.size() && collectibleCollected[i])
                continue;

            coinShape.setPosition(collectibles[i]);
            target.draw(coinShape);
        }
        if (powerupTextureLoaded) {
            for (std::size_t i = 0; i < powerups.size(); ++i) {
                if (i < powerupCollected.size() && powerupCollected[i])
                    continue;

                const float bobOffset = std::sin(powerupAnimTime * powerupBobSpeed + static_cast<float>(i) * 0.6f) * powerupBobAmplitude;
                const float pulse = 1.f + 0.08f * std::sin(powerupAnimTime * powerupPulseSpeed + static_cast<float>(i));
                powerupSprite.setScale(powerupBaseScaleX * pulse, powerupBaseScaleY * pulse);
                const float glow = 0.8f + 0.2f * std::sin(powerupAnimTime * powerupPulseSpeed + static_cast<float>(i) * 1.4f);
                const sf::Uint8 alpha = static_cast<sf::Uint8>(200 + 55 * glow);
                powerupSprite.setColor(sf::Color(255, 255, 255, alpha));
                powerupSprite.setPosition(sf::Vector2f(powerups[i].x, powerups[i].y + bobOffset));


                target.draw(powerupSprite);
            }
        }
        else {
            sf::RectangleShape powerupShape(sf::Vector2f(static_cast<float>(tileSize) * 0.8f, static_cast<float>(tileSize) * 0.8f));
            powerupShape.setFillColor(sf::Color(220, 80, 80));
            powerupShape.setOrigin(powerupShape.getSize() / 2.f);
            for (std::size_t i = 0; i < powerups.size(); ++i) {
                if (i < powerupCollected.size() && powerupCollected[i])
                    continue;

                powerupShape.setPosition(powerups[i]);
                target.draw(powerupShape);
            }
        

        }


        // Render goals as highlighted tiles
        sf::RectangleShape goalShape(sf::Vector2f(static_cast<float>(tileSize), static_cast<float>(tileSize)));
        goalShape.setFillColor(sf::Color(100, 200, 255, 180));
        for (const auto& tile : goalTiles) {
            goalShape.setPosition(static_cast<float>(tile.x * tileSize), static_cast<float>(tile.y * tileSize));
            target.draw(goalShape);
        }


    }

    bool isSolid(int x, int y) const {
        if (y < 0 || y >= static_cast<int>(tiles.size())) return false;
        if (x < 0 || x >= static_cast<int>(tiles[y].size())) return false;
        return tiles[y][x] > 0;
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
    std::vector<sf::Vector2f> getEnemySpawnPoints() const {
        std::vector<sf::Vector2f> points;
        points.reserve(enemySpawnTiles.size());
        for (const auto& tile : enemySpawnTiles) {
            points.emplace_back(static_cast<float>(tile.x * tileSize), static_cast<float>(tile.y * tileSize));
            
        }
        return points;

    }

    int getCollectibleCount() const {
        return static_cast<int>(collectibles.size());
    }
    void resetCollectibles() {
        collectibleCollected.assign(collectibleCollected.size(), false);

    }
    int getPowerCount() const {
        return static_cast<int>(powerups.size());
    }
    void resetPowerups() {
        powerupCollected.assign(powerupCollected.size(), false);
    }

    int getCollectedCount() const {
        int count = 0;
        for (bool taken : collectibleCollected) {
            if (taken) ++count;
        }
        return count;
    }

    int collectIfOverlapping(const sf::FloatRect& bounds) {
        int collectedCount = 0;
        for (std::size_t i = 0; i < collectibles.size() && i < collectibleCollected.size(); ++i) {
            if (collectibleCollected[i])
                continue;

            if (bounds.contains(collectibles[i])) {
                collectibleCollected[i] = true;
                ++collectedCount;

            }
        }
        return collectedCount;
    }
    int collectPowerupIfOverlapping(const sf::FloatRect& bounds) {
        int collectedCount = 0;
        for (std::size_t i = 0; i < powerups.size() && i < powerupCollected.size(); ++i) {
            if (powerupCollected[i])
                continue;
            if (bounds.contains(powerups[i])) {
                powerupCollected[i] = true;
                ++collectedCount;

            }
        }
        return collectedCount;
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
    private:
        static int tileIndexFromChar(char c) {
            if (std::isdigit(static_cast<unsigned char>(c))) {
                return c - '0';

            }
            if (c >= 'A' && c <= 'Z') {
                return c - '0';
            }
            if (c >= 'a' && c <= 'z') {
                return 36 + (c - 'a');

            }
            return 0;
        }
};
