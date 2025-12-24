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
    bool pauseHeld = false;
    bool paused = false;
    void run();

    Entity* CreateEntity() {
        return scene.createEntity();
    }
    sf::View camera;

private:
    Window window;   // Our new window system!
    Scene scene;  // The scene managing entities
 





    void processEvents();
    void update(float dt);
    void render();

    void clampCameraToLevel();
    void resetPlayerIfFallen();
    void handleCollectibles();
    void checkGoalReached();
    void resetLevelState();

};
