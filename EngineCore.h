#pragma once
#include "Window.h"
#include "Scene.h"
#include "Tilemap.h"


class EngineCore {
public:
    EngineCore();
    Tilemap tilemap;
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
};
