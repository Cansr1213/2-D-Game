#include "EngineCore.h"
#include "Window.h"
#include "Scene.h"
#include "Entity.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "MovementComponent.h"
#include "PhysicsComponent.h"
#include "Tilemap.h"
#include "AnimationComponent.h"
#include <iostream>
#include <filesystem>   // REQUIRED for current_path()

EngineCore::EngineCore()
    : window("My 2D Game Engine", 800, 600),
    tilemap(1, 1)
{
    camera = window.getRenderWindow().getDefaultView();

    // ✅ TILEMAP MUST USE TILE TEXTURE (NOT PLAYER)
    tilemap.loadTileset("Assets/platform.png", 32, 32);
    tilemap.loadFromFile("Assets/level1.txt");

    // ✅ CREATE ONE PLAYER ENTITY
    Entity* player = scene.createEntity();

    TransformComponent* transform =
        player->addComponent<TransformComponent>(100.f, 100.f);

    SpriteComponent* sprite =
        player->addComponent<SpriteComponent>("Assets/player.png", transform);

    player->addComponent<MovementComponent>(transform, &tilemap);
    player->addComponent<PhysicsComponent>(transform, &tilemap);
    player->addComponent<AnimationComponent>(sprite, 64, 64, 6, 0.12f);
}


void EngineCore::run() {
    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        processEvents();
        update(dt);
        render();
    }
}

void EngineCore::processEvents() {
    window.processEvents();
}

void EngineCore::update(float dt) {
    // -- CAMERA FOLLOW LOGIC -- //
    auto& ents = scene.getEntities();
    if (!ents.empty()) {
        TransformComponent* t = ents[0]->getComponent<TransformComponent>();
        if (t) {
            camera.setCenter(t->position);

        }

    }

    scene.update(dt);



}

void EngineCore::render() {
    window.beginDraw();

    // Camera view
    window.getRenderWindow().setView(camera);

    // Draw tilemap BEFORE entities 
    tilemap.render(window.getRenderWindow());

    // Draw entities
    scene.render(window.getRenderWindow());

    // Rest view
    window.getRenderWindow().setView(window.getRenderWindow().getDefaultView());


    window.endDraw();
}
