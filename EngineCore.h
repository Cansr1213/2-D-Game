#pragma once
#include "Window.h"
#include "Scene.h"
#include "Tilemap.h"
#include <SFML/Graphics.hpp>

class EngineCore {
public:
    EngineCore();
    Tilemap tilemap;
    Entity* player = nullptr;
    sf::Vector2f playerSpawn{ 100.f, 100.f };
    int collectedCoins = 0;
    bool levelComplete = false;
    bool resetHeld = false;
    bool pausedHeld = false;
    bool paused = false;
    void run();

    Entity* CreateEntity() {
        return scene.createEntity();
    }
    sf::View camera;

private:
    Window window;   // Our new window system!
    Scene scene;  // The scene managing entities
 
    sf::Font uiFont;
    sf::Text coinText;
    sf::Text pauseText;
    sf::Text goalText;
    sf::Text controlsText;
    float goalMessageTimer = 0.f;
    const float goalMessageDuration = 2.5f;





    void processEvents();
    void update(float dt);
    void render();

    void clampCameraToLevel();
    void respawnPlayer();
    void resetPlayerIfFallen();
    void handleCollectibles();
    void checkGoalReached();
    void handleEnemyCollisions();
    void resetLevelState();

};
