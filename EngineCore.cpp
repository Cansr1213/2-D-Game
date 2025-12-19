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
#include <algorithm>


EngineCore::EngineCore()
    : window("My 2D Game Engine", 800, 600),
    tilemap()
{
    camera = window.getRenderWindow().getDefaultView();

    // ✅ TILEMAP MUST USE TILE TEXTURE (NOT PLAYER)
    tilemap.loadTileset("Assets/platform.png", 32, 32);
    tilemap.loadFromFile("Assets/level1.txt");
    if (tilemap.hasSpawnPoint()) {
        playerSpawn = tilemap.getSpawnPoint();
    }



    // ✅ CREATE ONE PLAYER ENTITY
    player = scene.createEntity();

    TransformComponent* transform = player->addComponent<TransformComponent>(playerSpawn.x, playerSpawn.y);

    SpriteComponent* sprite =
        player->addComponent<SpriteComponent>("Assets/player.png", transform);

    player->addComponent<MovementComponent>(transform, &tilemap);
    player->addComponent<PhysicsComponent>(transform, &tilemap);
    player->addComponent<AnimationComponent>(sprite, 47, 0, 6, 0.12f);
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
    if (player) {
        TransformComponent* t = player->getComponent<TransformComponent>();
        if (t) {
            camera.setCenter(t->position);
        }
    

    }

    resetPlayerIfFallen();
    clampCameraToLevel();
    handleCollectibles();
    checkGoalReached();
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

void EngineCore::clampCameraToLevel() {
    const int levelWidth = tilemap.getPixelWidth();
    const int levelHeight = tilemap.getPixelHeight();

    if (levelWidth == 0 || levelHeight == 0)
        return;

    sf::Vector2f center = camera.getCenter();
    const sf::Vector2f size = camera.getSize();

    const float halfW = size.x / 2.f;
    const float halfH = size.y / 2.f;

    const float minX = halfW;
    const float minY = halfH;
    const float maxX = static_cast<float>(levelWidth) - halfW;
    const float maxY = static_cast<float>(levelHeight) - halfH;

    center.x = std::clamp(center.x, minX, std::max(minX, maxX));
    center.y = std::clamp(center.y, minY, std::max(minY, maxY));

    camera.setCenter(center);
}

void EngineCore::resetPlayerIfFallen() {
    if (!player)
        return;

    TransformComponent* transform = player->getComponent<TransformComponent>();
    PhysicsComponent* physics = player->getComponent<PhysicsComponent>();

    if (!transform)
        return;

    const float fallThreshold = static_cast<float>(tilemap.getPixelHeight()) + 200.f;

    if (transform->position.y > fallThreshold) {
        transform->position = playerSpawn;

        if (physics) {
            physics->velocityY = 0.f;
            physics->onGround = false;
        }

        camera.setCenter(playerSpawn);
    }
}

void EngineCore::handleCollectibles() {
    if (!player)
        return;

    TransformComponent* transform = player->getComponent<TransformComponent>();
    MovementComponent* movement = player->getComponent<MovementComponent>();

    if (!transform)
        return;

    const float width = movement ? movement->colliderWidth : 32.f;
    const float height = movement ? movement->colliderHeight : 48.f;

    const sf::FloatRect bounds(transform->position.x, transform->position.y, width, height);

    if (tilemap.collectIfOverlapping(bounds)) {
        collectedCoins = tilemap.getCollectedCount();
        std::cout << "Collected coin " << collectedCoins << " / " << tilemap.getCollectibleCount() << "\n";
    }
}

void EngineCore::checkGoalReached() {
    if (!player || levelComplete)
        return;

    TransformComponent* transform = player->getComponent<TransformComponent>();
    MovementComponent* movement = player->getComponent<MovementComponent>();

    if (!transform)
        return;

    const float width = movement ? movement->colliderWidth : 32.f;
    const float height = movement ? movement->colliderHeight : 48.f;
    const sf::FloatRect bounds(transform->position.x, transform->position.y, width, height);

    if (tilemap.reachedGoal(bounds)) {
        levelComplete = true;
        std::cout << "Goal reached! Coins collected: " << collectedCoins << " / "
            << tilemap.getCollectibleCount() << "\n";

        transform->position = playerSpawn;

        if (PhysicsComponent* physics = player->getComponent<PhysicsComponent>()) {
            physics->velocityY = 0.f;
            physics->onGround = false;
        }

        camera.setCenter(playerSpawn);
    }
}