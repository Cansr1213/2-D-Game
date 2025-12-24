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
#include <SFML/Graphics.hpp>


EngineCore::EngineCore()
    : window("My 2D Game Engine", 800, 600),
    tilemap()
{
    camera = window.getRenderWindow().getDefaultView();

    if (!uiFont.loadFromFile("Assets/DejaVuSans.ttf")) {
        std::cerr << "Failed to load UI font Assets/DejaVuSans.tff\n";

    }
    coinText.setFont(uiFont);
    coinText.setCharacterSize(20);
    coinText.setFillColor(sf::Color::White);
    coinText.setPosition(16.f, 12.f);

    pauseText.setFont(uiFont);
    pauseText.setCharacterSize(20);
    pauseText.setFillColor(sf::Color::Yellow);
    pauseText.setPosition(16.f, 40.f);

    goalText.setFont(uiFont);
    goalText.setCharacterSize(28);
    goalText.setFillColor(sf::Color::Green);
   

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
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();

        }
        if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                window.close();

            }
            if (event.key.code == sf::Keyboard::P) {
                paused = !paused;
                std::cout << (!paused ? "Game paused\n" : "Game resume\n");
            }
        }
    }

}

void EngineCore::update(float dt) {
    // -- CAMERA FOLLOW LOGIC -- //
    if (player) {
        TransformComponent* t = player->getComponent<TransformComponent>();
        if (t) {
            camera.setCenter(t->position);
        }
    

    }
    
    
    
    const bool resetPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::R);
    if (resetPressed && !resetHeld) {
        resetLevelState();
        
    }
    resetHeld = resetPressed;

    if (paused) {
        return;

    }


    resetPlayerIfFallen();
    clampCameraToLevel();
    handleCollectibles();
    checkGoalReached();
    scene.update(dt);
    if (levelComplete) {
        goalMessageTimer -= dt;
        if (goalMessageTimer <= 0.f) {
            resetLevelState();

        }
    }



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

    coinText.setString("Coins: " + std::to_string(collectedCoins) + " / " + std::to_string((tilemap.getCollectibleCount())));
    pauseText.setString(paused ? "Paused" : "");

    if (goalMessageTimer > 0.f) {
        goalText.setString("Goal reached!");
        const sf::FloatRect bounds = goalText.getLocalBounds();
        const sf::Vector2u  windowSize = window.getRenderWindow().getSize();
        goalText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
        goalText.setPosition(static_cast<float>(windowSize.x) / 2.f, 100.f);
        window.getRenderWindow().draw(goalText);

    }
    window.getRenderWindow().draw(coinText);
    window.getRenderWindow().draw(pauseText);


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
        goalMessageTimer = goalMessageDuration;
        std::cout << "Goal reached! Coins collected: " << collectedCoins << " / "
            << tilemap.getCollectibleCount() << "\n";

        
    }
}
void EngineCore::resetLevelState() {
    levelComplete = false;
    goalMessageTimer = 0.f;
    collectedCoins = 0;
    tilemap.resetCollectibles();
    if (!player)
        return;
    if (TransformComponent* transform = player->getComponent<TransformComponent>()) {
        transform->position = playerSpawn;

    }
    if (PhysicsComponent* physics = player->getComponent<PhysicsComponent>()) {
        physics->velocityY = 0.f;
        physics->onGround = false;

    }
    camera.setCenter(playerSpawn);
}


        