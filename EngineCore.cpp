#include "EngineCore.h"
#include "Window.h"
#include "Scene.h"
#include "Entity.h"
#include "TransformComponent.h"
#include "SpriteComponent.h"
#include "MovementComponent.h"
#include "PhysicsComponent.h"
#include "EnemyComponent.h"
#include "Tilemap.h"
#include "AnimationComponent.h"
#include <iostream>
#include <filesystem>   // REQUIRED for current_path()
#include <algorithm>
#include <cmath>
#include <SFML/Graphics.hpp>

namespace {
    std::filesystem::path findAssetsRoot() {
        std::filesystem::path current = std::filesystem::current_path();
        for (int depth = 0; depth < 5; ++depth) {
            std::filesystem::path candidate = current / "Assets";
            if (std::filesystem::exists(candidate) && std::filesystem::is_directory(candidate)) {
                return candidate;
            }
            if (!current.has_parent_path()) {
                break;
            }
            current = current.parent_path();
        }
        return {};
    }
}


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

    controlsText.setFont(uiFont);
    controlsText.setCharacterSize(16);
    controlsText.setFillColor(sf::Color(220, 220, 220));
    controlsText.setPosition(16.f, 500.f);

   

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

    for (const auto& enemySpawn : tilemap.getEnemySpawnPoints()) {
        Entity* goomba = scene.createEntity();
        TransformComponent* goombaTransform =
            goomba->addComponent<TransformComponent>(enemySpawn.x, enemySpawn.y);
        SpriteComponent* goombaSprite =
            goomba->addComponent<SpriteComponent>("Assets/nathaniel.png", goombaTransform);

        goombaSprite->getSprite().setTextureRect(sf::IntRect(0, 0, 32, 32));
        goomba->addComponent<PhysicsComponent>(goombaTransform, &tilemap, 32.f, 32.f, false);
        goomba->addComponent<EnemyComponent>(goombaTransform, &tilemap, 32.f, 32.f);
        goomba->addComponent<AnimationComponent>(goombaSprite, 47, 0, 6, 0.12f);
    }






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
    handleEnemyCollisions();
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
    controlsText.setString("Move: A/D Jump: Space Reset: R Pause: P");
    window.getRenderWindow().draw(controlsText);


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
    
    center.x = std::round(center.x);
    center.y = std::round(center.y);


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
        

        respawnPlayer();

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
void EngineCore::respawnPlayer() {
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

void EngineCore::handleEnemyCollisions() {
    if (!player)
        return;

    TransformComponent* playerTransform = player->getComponent<TransformComponent>();
    PhysicsComponent* playerPhysics = player->getComponent<PhysicsComponent>();
    MovementComponent* playerMovement = player->getComponent<MovementComponent>();

    if (!playerTransform || !playerPhysics)
        return;

    const float playerWidth = playerMovement ? playerMovement->colliderWidth : 32.f;
    const float playerHeight = playerMovement ? playerMovement->colliderHeight : 48.f;
    const sf::FloatRect playerBounds(
        playerTransform->position.x,
        playerTransform->position.y,
        playerWidth,
        playerHeight);

    for (auto& entity : scene.getEntities()) {
        EnemyComponent* enemy = entity->getComponent<EnemyComponent>();
        if (!enemy || !enemy->alive)
            continue;

        TransformComponent* enemyTransform = entity->getComponent<TransformComponent>();
        if (!enemyTransform)
            continue;

        const sf::FloatRect enemyBounds(
            enemyTransform->position.x,
            enemyTransform->position.y,
            enemy->colliderWidth,
            enemy->colliderHeight);

        if (!playerBounds.intersects(enemyBounds))
            continue;

        const float playerBottom = playerBounds.top + playerBounds.height;
        const float enemyTop = enemyBounds.top;
        const bool stomp = playerBottom <= enemyTop + 5.f && playerPhysics->velocityY > 0.f;

        if (stomp) {
            enemy->alive = false;
            enemy->deathTimer = enemy->deathDelay;
            if (PhysicsComponent* enemyPhysics = entity->getComponent<PhysicsComponent>()) {
                enemyPhysics->velocityY = 0.f;
                enemyPhysics->onGround = true;


            }
            if (SpriteComponent* sprite = entity->getComponent<SpriteComponent>()) {
                const float currentXScale = sprite->getSprite().getScale().x;
                sprite->getSprite().setScale(currentXScale, 0.5f);

            }
            playerPhysics->velocityY = -250.f;
            playerPhysics->onGround = false;

        }
        else {
            respawnPlayer();

        }
    }

}
void EngineCore::resetLevelState() {
    levelComplete = false;
    goalMessageTimer = 0.f;
    collectedCoins = 0;
    tilemap.resetCollectibles();
    respawnPlayer();

}

        