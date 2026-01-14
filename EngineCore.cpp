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
#include <exception>
#include <filesystem>   // REQUIRED for current_path()
#include <fstream>
#include <algorithm>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


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
    : window("Alice Wild Adventure", 1920, 1080),
    tilemap()
{
    camera = window.getRenderWindow().getDefaultView();

    if (!uiFont.loadFromFile("Assets/DejaVuSans.ttf")) {
        std::cerr << "Failed to load UI font Assets/DejaVuSans.tff\n";

    }
    livesText.setFont(uiFont);
    livesText.setCharacterSize(20);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(16.f, 12.f);

    coinText.setFont(uiFont);
    coinText.setCharacterSize(20);
    coinText.setFillColor(sf::Color::White);
    coinText.setPosition(150.f, 12.f);

    scoreText.setFont(uiFont);
    scoreText.setCharacterSize(18);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(16.f, 40.f);

    pauseText.setFont(uiFont);
    pauseText.setCharacterSize(20);
    pauseText.setFillColor(sf::Color::Yellow);
    pauseText.setPosition(16.f, 40.f);

    goalText.setFont(uiFont);
    goalText.setCharacterSize(28);
    goalText.setFillColor(sf::Color::Green);

    gameOverText.setFont(uiFont);
    gameOverText.setCharacterSize(32);
    gameOverText.setFillColor(sf::Color::Red);

    controlsText.setFont(uiFont);
    controlsText.setCharacterSize(16);
    controlsText.setFillColor(sf::Color(220, 220, 220));
    controlsText.setPosition(16.f, 500.f);

    powerText.setFont(uiFont);
    powerText.setCharacterSize(20);
    powerText.setFillColor(sf::Color::White);
    powerText.setPosition(16.f, 66.f);

    reserveText.setFont(uiFont);
    reserveText.setCharacterSize(18);
    reserveText.setFillColor(sf::Color::White);
    reserveText.setPosition(16.f, 92.f);

    levelText.setFont(uiFont);
    levelText.setCharacterSize(20);
    levelText.setFillColor(sf::Color::White);

    startMenuTitleText.setFont(uiFont);
    startMenuTitleText.setCharacterSize(40);
    startMenuTitleText.setFillColor(sf::Color::Cyan);
    startMenuTitleText.setString("Alice Wild Adventure");

    startMenuPromptText.setFont(uiFont);
    startMenuPromptText.setCharacterSize(20);
    startMenuPromptText.setFillColor(sf::Color::Yellow);
    startMenuPromptText.setString("Press Enter to Start");

    beginText.setFont(uiFont);
    beginText.setCharacterSize(28);
    beginText.setFillColor(sf::Color::White);
    beginText.setString("Begin!");

    worldMapTitleText.setFont(uiFont);
    worldMapTitleText.setCharacterSize(36);
    worldMapTitleText.setFillColor(sf::Color::Cyan);
    worldMapTitleText.setString("World Map");

    worldMapPromptText.setFont(uiFont);
    worldMapPromptText.setCharacterSize(18);
    worldMapPromptText.setFillColor(sf::Color::Yellow);
    worldMapPromptText.setString("Use Left/Right to choose a level, Enter to play");

    worldMapLevelText.setFont(uiFont);
    worldMapLevelText.setCharacterSize(22);
    worldMapLevelText.setFillColor(sf::Color::White);



    // ✅ TILEMAP MUST USE TILE TEXTURE (NOT PLAYER)
    try {
        tilemap.loadTileset("Assets/tileset.png", 16, 16);
    }
    catch (const std::exception& ex) {
        std::cerr << "Failed to load tile image  Assets/tileset.png (" << ex.what() << "). Falling back to Assets/platform.png\n";
        tilemap.loadTileset("Assets/platform.png", 32, 32);
    }


    if (backgroundMusic.openFromFile("Assets/music.wav")) {
        backgroundMusic.setLoop(true);
        backgroundMusic.setVolume(40.f);
        backgroundMusic.play();
    }
    else {
        std::cerr << "Failed to load background music  Assets/music.wav\n";
    }
    setupLevelList();
    loadProgress();
    currentLevelIndex = std::clamp(currentLevelIndex, 0, static_cast<int>(levels.size()) - 1);
    selectedLevelIndex = std::clamp(selectedLevelIndex, 0, static_cast<int>(levels.size()) - 1);
    loadLevel(currentLevelIndex);
    updateWorldMapText();




}
void EngineCore::setupLevelList() {
    levels = {
        {1, 1, "Assets/level1.txt"},
        {1, 2, "Assets/level2.txt"},
        {1, 3, "Assets/level3.txt"}
    };
    if (levels.empty()) {
        levels.push_back({ 1, 1, "Assets/level1.txt" });
    }
}
void EngineCore::loadProgress() {
    std::ifstream in("save.dat");
    if (!in.is_open()) {
        currentLevelIndex = 0;
        selectedLevelIndex = 0;
        maxUnlockedLevelIndex = 0;
        return;

    }
    int current = 0;
    int unlocked = 0;
    in >> current >> unlocked;
    if (!in.fail()) {
        currentLevelIndex = std::clamp(current, 0, static_cast<int>(levels.size()) - 1);
        maxUnlockedLevelIndex = std::clamp(unlocked, 0, static_cast<int>(levels.size()) - 1);
        selectedLevelIndex = std::min(currentLevelIndex, maxUnlockedLevelIndex);


    }
}
void EngineCore::saveProgress() {
    std::ofstream out("save.dat", std::ios::trunc);
    if (!out.is_open()) {
        return;

    }
    out << currentLevelIndex << " " << maxUnlockedLevelIndex;

}
void EngineCore::updateWorldMapText() {
    if (levels.empty()) {
        worldMapLevelText.setString("No levels available.");
        return;

    }
    std::string list;
    const int maxIndex = std::min(maxUnlockedLevelIndex, static_cast<int>(levels.size()) - 1);
    for (int i = 0; i < static_cast<int>(levels.size()); ++i) {
        const bool locked = i > maxIndex;
        const bool selected = i == selectedLevelIndex;
        const auto& levelInfo = levels[i];
        list += selected ? "> " : " ";
        list += std::to_string(levelInfo.world) + "-" + std::to_string(levelInfo.level);
        if (locked) {
            list += " (Locked)";

        }
        list += "\n";

    }
    worldMapLevelText.setString(list);
}
void EngineCore::loadLevel(int levelIndex) {
    if (levels.empty()) {
        return;
    }
    const int safeIndex = std::clamp(levelIndex, 0, static_cast<int>(levels.size()) - 1);
    currentLevelIndex = safeIndex;
    selectedLevelIndex = safeIndex;
    currentWorld = levels[safeIndex].world;
    currentLevel = levels[safeIndex].level;
    tilemap.loadFromFile(levels[safeIndex].file);
    if (tilemap.hasSpawnPoint()) {
        playerSpawn = tilemap.getSpawnPoint();

    }
    scene.clear();
    
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
        goomba->addComponent<AnimationComponent>(goombaSprite, 47, 0, 6, 0.20f);
    }
    levelComplete = false;
    goalMessageTimer = 0.f;
    collectedCoins = 0;
    playerDying = false;
    invincible = false;
    invincibilityTimer = false;
    powerupFlashTimer = 0.f;
    tilemap.resetCollectibles();
    tilemap.resetPowerups();
    reservePowerup.reset();
    setPlayerPowerState(PlayerPowerState::Small);
    respawnPlayer();
}
void EngineCore::enterWorldMap() {
    gameState = GameState::WorldMap;
    updateWorldMapText();

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
        if (gameState == GameState::StartMenu && !startTransition && event.key.code == sf::Keyboard::Enter) {
            startTransition = true;
            startTransitionTimer = startTransitionDuration;
           
            resetGameState();

        }
        if (gameState == GameState::WorldMap) {
            if (event.key.code == sf::Keyboard::Left) {
                selectedLevelIndex = std::max(0, selectedLevelIndex - 1);
                updateWorldMapText();

            }
            if (event.key.code == sf::Keyboard::Right) {
                const int maxIndex = std::min(maxUnlockedLevelIndex, static_cast<int>(levels.size()) - 1);
                selectedLevelIndex = std::min(maxIndex, selectedLevelIndex + 1);
                updateWorldMapText();
            }
            if (event.key.code == sf::Keyboard::Enter) {
                if (selectedLevelIndex <= maxUnlockedLevelIndex) {
                    loadLevel(selectedLevelIndex);
                    gameState = GameState::Playing;
                    paused = false;
                }
            }
        }
        if (gameState == GameState::Playing && event.key.code == sf::Keyboard::P) {
            paused = !paused;
            std::cout << (!paused ? "Game paused\n" : "Game continue\n");

        }

         
        }
    }

}

void EngineCore::update(float dt) {
   
    // -- CAMERA FOLLOW LOGIC -- //
    if (gameState == GameState::StartMenu) {
        if (player) {
            if (SpriteComponent* sprite = player->getComponent<SpriteComponent>()) {
                sprite->setVisible(false);
            }
        }
        if (startTransition) {
            startTransitionTimer = std::max(0.f, startTransitionTimer - dt);
            if (startTransitionTimer <= 0.f) {
                startTransition = false;

                enterWorldMap();
            }
        }
        return;
    }
    if (gameState == GameState::WorldMap) {
        if (player) {
            if (SpriteComponent* sprite = player->getComponent<SpriteComponent>()) {
                sprite->setVisible(false);
            }
        }
        return;
    

    }
    if (player) {
        if (SpriteComponent* sprite = player->getComponent<SpriteComponent>()) {
            sprite->setVisible(true);
        }
    }
    
    
    
    const bool resetPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::R);
    if (resetPressed && !resetHeld) {
        if (gameOver) {
            resetGameState();

        }
        else {
            resetLevelState();

        }
        
    }
    resetHeld = resetPressed;

    if (paused) {
        return;

    }
    if (gameOver) {
        return;

    }
    const bool reservePressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
    if (reservePressed && !reserveHeld) {
        handleReserveActivation();
    }
    reserveHeld = reservePressed;

    tilemap.update(dt);
	updateInvincibility(dt);
    updatePowerupFlash(dt);
    if (playerDying) {
        scene.update(dt);
        updatePlayerDeath(dt);
        clampPlayerToLevel();
        updateCameraFollow();
        clampCameraToLevel();
        return;
    }
    resetPlayerIfFallen();
    handleCollectibles();
    handlePowerups();
    checkGoalReached();
    scene.update(dt);
    applyGlidePhysics();
    clampPlayerToLevel();
    updateCameraFollow();
    clampCameraToLevel();
    handleEnemyCollisions();
    if (levelComplete) {
        goalMessageTimer -= dt;
        if (goalMessageTimer <= 0.f) {
            const int nextIndex = currentLevelIndex + 1;
            if (nextIndex < static_cast<int>(levels.size())) {
                maxUnlockedLevelIndex = std::max(maxUnlockedLevelIndex, nextIndex);
                currentLevelIndex = nextIndex;
                selectedLevelIndex = currentLevelIndex;
            }
            saveProgress();
            enterWorldMap();


        }
    }



}

void EngineCore::render() {
    window.beginDraw();

    // Camera view
    if (gameState == GameState::StartMenu && startTransition) {
        renderPixelatedScene();
    }
    else if (gameState == GameState::Playing) {
        renderScene(window.getRenderWindow());
    }
    else {
        window.getRenderWindow().clear(sf::Color::Black);
    }

    // Draw tilemap BEFORE entities 
    window.getRenderWindow().setView(window.getRenderWindow().getDefaultView());

    // Draw entities
    if (gameState == GameState::Playing) {
        livesText.setString("Lives: " + std::to_string(lives));
        coinText.setString("Coins: " + std::to_string(coinBank));
        scoreText.setString("Score: " + std::to_string(score));
        pauseText.setString(paused ? "Paused" : "");
        levelText.setString(std::to_string(currentWorld) + "-" + std::to_string(currentLevel) + " World");
        const sf::FloatRect levelBounds = levelText.getLocalBounds();
        const sf::Vector2u windowSize = window.getRenderWindow().getSize();
        levelText.setPosition(static_cast<float>(windowSize.x) - levelBounds.width - 16.f, 12.f);

        if (goalMessageTimer > 0.f) {
            goalText.setString("Goal reached!");
            const sf::FloatRect bounds = goalText.getLocalBounds();
            const sf::Vector2u  windowSize = window.getRenderWindow().getSize();
            goalText.setOrigin(bounds.left + bounds.width / 2.f, bounds.top + bounds.height / 2.f);
            goalText.setPosition(static_cast<float>(windowSize.x) / 2.f, 100.f);
            window.getRenderWindow().draw(goalText);
        }
        window.getRenderWindow().draw(coinText);
        window.getRenderWindow().draw(scoreText);
        window.getRenderWindow().draw(livesText);
        window.getRenderWindow().draw(pauseText);
        window.getRenderWindow().draw(levelText);
        powerText.setString("Power: " + toPowerupLabel(currentPowerState));
        const std::string reserveLabel = reservePowerup ? toPowerupLabel(*reservePowerup) : "Empty";
        reserveText.setString("Reserve (Q): " + reserveLabel);
        window.getRenderWindow().draw(powerText);
        window.getRenderWindow().draw(reserveText);
        controlsText.setString("Move: A/D Jump: Space Run: Shift Reserve: Q Reset: R Pause: P");
        window.getRenderWindow().draw(controlsText);

        if (gameOver) {
            gameOverText.setString("Gameover - Press R to Restart");
            const sf::FloatRect bounds = gameOverText.getLocalBounds();
            const sf::Vector2u windowSize = window.getRenderWindow().getSize();
            gameOverText.setPosition(static_cast<float>(windowSize.x) / 2.f, 140.f);
            window.getRenderWindow().draw(gameOverText);
        }
    }

    if (gameState == GameState::WorldMap) {
        const sf::Vector2u menuSize = window.getRenderWindow().getSize();
        const sf::FloatRect titleBounds = worldMapTitleText.getLocalBounds();
        worldMapTitleText.setOrigin(titleBounds.left + titleBounds.width / 2.f, titleBounds.top + titleBounds.height / 2.f);
        worldMapTitleText.setPosition(static_cast<float>(menuSize.x) / 2.f, 160.f);
        worldMapPromptText.setPosition(120.f, 240.f);
        worldMapLevelText.setPosition(160.f, 280.f);
        window.getRenderWindow().draw(worldMapTitleText);
        window.getRenderWindow().draw(worldMapPromptText);
        window.getRenderWindow().draw(worldMapLevelText);
    }
    if (gameState == GameState::StartMenu && !startTransition) {
        const sf::FloatRect titleBounds = startMenuTitleText.getLocalBounds();
        const sf::FloatRect promptBounds = startMenuPromptText.getLocalBounds();
        const sf::Vector2u menuSize = window.getRenderWindow().getSize();
        startMenuTitleText.setOrigin(titleBounds.left + titleBounds.width / 2.f, titleBounds.top + titleBounds.height / 2.f);
        startMenuTitleText.setPosition(static_cast<float>(menuSize.x) / 2.f, 200.f);
        startMenuPromptText.setOrigin(promptBounds.left + promptBounds.width / 2.f, promptBounds.top + promptBounds.height / 2.f);
        startMenuPromptText.setPosition(static_cast<float>(menuSize.x) / 2.f, 280.f);
        window.getRenderWindow().draw(startMenuTitleText);
        window.getRenderWindow().draw(startMenuPromptText);



    

    }
    if (startTransition && startTransitionTimer <= beginTextDuration) {
        const sf::Vector2u windowSize = window.getRenderWindow().getSize();
        const sf::FloatRect beginBounds = beginText.getLocalBounds();
        beginText.setOrigin(beginBounds.left + beginBounds.width / 2.f, beginBounds.top + beginBounds.height / 2.f);
        beginText.setPosition(static_cast<float>(windowSize.x) / 2.f, 220.f);
        window.getRenderWindow().draw(beginText);
    }


    window.endDraw();

}
void EngineCore::renderScene(sf::RenderTarget& target) {
    const sf::View previousView = target.getView();
    target.setView(camera);
    tilemap.render(target);
    scene.render(target);
    target.setView(previousView);
}

void EngineCore::renderPixelatedScene() {
    const sf::Vector2u windowSize = window.getRenderWindow().getSize();
    const float progress = 1.f - (startTransitionTimer / std::max(startTransitionDuration, 0.001f));
    const float startPixelSize = 24.f;
    const float pixelSize = std::max(1.f, startPixelSize * (1.f - progress) + 1.f);
    const unsigned int textureWidth = std::max(1u, static_cast<unsigned int>(windowSize.x / pixelSize));
    const unsigned int textureHeight = std::max(1u, static_cast<unsigned int>(windowSize.y / pixelSize));

    if (textureWidth != pixelateTextureSize.x || textureHeight != pixelateTextureSize.y) {
        pixelateTexture.create(textureWidth, textureHeight);
        pixelateTexture.setSmooth(false);
        pixelateTextureSize = { textureWidth, textureHeight };
    }

    pixelateTexture.clear(sf::Color::Black);
    renderScene(pixelateTexture);
    pixelateTexture.display();

    pixelateSprite.setTexture(pixelateTexture.getTexture(), true);
    pixelateSprite.setPosition(0.f, 0.f);
    pixelateSprite.setScale(
        static_cast<float>(windowSize.x) / static_cast<float>(textureWidth),
        static_cast<float>(windowSize.y) / static_cast<float>(textureHeight));
    window.getRenderWindow().draw(pixelateSprite);
}
void EngineCore::updateCameraFollow() {
    if (!player) {
        return;
    }

    TransformComponent* transform = player->getComponent<TransformComponent>();
    MovementComponent* movement = player->getComponent<MovementComponent>();

    if (!transform) {
        return;
    }

    const float width = movement ? movement->colliderWidth : 32.f;
    const float height = movement ? movement->colliderHeight : 48.f;
    const sf::Vector2f playerCenter(
        transform->position.x + width / 2.f,
        transform->position.y + height / 2.f);

    sf::Vector2f center = camera.getCenter();
    const sf::Vector2f size = camera.getSize();
    const float halfW = size.x / 2.f;
    const float halfH = size.y / 2.f;

    const float deadzoneX = size.x * 0.25f;
    const float deadzoneY = size.y * 0.25f;

    const float leftBound = center.x - halfW + deadzoneX;
    const float rightBound = center.x + halfW - deadzoneX;
    const float topBound = center.y - halfH + deadzoneY;
    const float bottomBound = center.y + halfH - deadzoneY;

    if (playerCenter.x < leftBound) {
        center.x = playerCenter.x + halfW - deadzoneX;
    }
    else if (playerCenter.x > rightBound) {
        center.x = playerCenter.x - halfW + deadzoneX;
    }

    if (playerCenter.y < topBound) {
        center.y = playerCenter.y + halfH - deadzoneY;
    }
    else if (playerCenter.y > bottomBound) {
        center.y = playerCenter.y - halfH + deadzoneY;
    }

    camera.setCenter(center);
}

void EngineCore::clampPlayerToLevel() {
    if (!player)
        return;

    TransformComponent* transform = player->getComponent<TransformComponent>();
    MovementComponent* movement = player->getComponent<MovementComponent>();

    if (!transform)
        return;

    const float width = movement ? movement->colliderWidth : 32.f;
    const float height = movement ? movement->colliderHeight : 48.f;
    const float maxX = static_cast<float>(tilemap.getPixelWidth()) - width;
    const float maxY = static_cast<float>(tilemap.getPixelHeight()) - height;

    transform->position.x = std::clamp(transform->position.x, 0.f, std::max(0.f, maxX));
    transform->position.y = std::clamp(transform->position.y, 0.f, std::max(0.f, maxY));
}

void EngineCore::startPlayerDeath() {
    if (!player)
        return;

    playerDying = true;
    deathTimer = deathDuration;

    if (TransformComponent* transform = player->getComponent<TransformComponent>()) {
        deathStartPosition = transform->position;
    }
    if (MovementComponent* movement = player->getComponent<MovementComponent>()) {
        movement->setCrouching(false);
        movement->enabled = false;
    }
    if (PhysicsComponent* physics = player->getComponent<PhysicsComponent>()) {
        physics->enabled = false;
        physics->velocityY = 0.f;
    }
    if (AnimationComponent* anim = player->getComponent<AnimationComponent>()) {
        anim->paused = true;
    }
}

void EngineCore::updatePlayerDeath(float dt) {
    if (!playerDying || !player)
        return;

    deathTimer = std::max(0.f, deathTimer - dt);
    const float progress = 1.f - (deathTimer / std::max(deathDuration, 0.001f));

    TransformComponent* transform = player->getComponent<TransformComponent>();
    SpriteComponent* sprite = player->getComponent<SpriteComponent>();
    MovementComponent* movement = player->getComponent<MovementComponent>();

    if (transform) {
        const float lift = std::sin(progress * 3.14159f) * deathFloatHeight;
        transform->position.y = deathStartPosition.y - lift;
    }
    if (sprite) {
        const sf::Vector2f currentScale = sprite->getSprite().getScale();
        const float baseX = (currentScale.x < 0.f) ? -1.f : 1.f;
        const float baseY = movement ? movement->baseScaleY : std::abs(currentScale.y);
        const float wobble = 1.f + 0.2f * std::sin(progress * 10.f);
        const float shrink = 1.f - 0.4f * progress;
        sprite->getSprite().setScale(baseX * wobble, baseY * shrink);
        sprite->getSprite().rotate(deathSpinSpeed * dt);
    }

    if (deathTimer <= 0.f) {
        playerDying = false;
        if (sprite) {
            sprite->getSprite().setRotation(0.f);
        }
        if (movement) {
            movement->enabled = true;
            movement->setCrouching(false);
        }
        if (PhysicsComponent* physics = player->getComponent<PhysicsComponent>()) {
            physics->enabled = true;
        }
        if (AnimationComponent* anim = player->getComponent<AnimationComponent>()) {
            anim->paused = false;
        }
        respawnPlayer();
    }
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
        

        loseLife();

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

    const int collectedNow = tilemap.collectIfOverlapping(bounds);
    if (collectedNow > 0) {

        collectedCoins = tilemap.getCollectedCount();
        score += collectedNow * coinScoreValue;
        coinBank += collectedNow;
        while (coinBank >= 100) {
            coinBank -= 100;
            lives += 1;
        }
        std::cout << "Collected coin " << collectedCoins << " / " << tilemap.getCollectibleCount() << "\n";
    }
}
void EngineCore::handlePowerups() {
    if (!player)
        return;
    TransformComponent* transform = player->getComponent<TransformComponent>();
    MovementComponent* movement = player->getComponent<MovementComponent>();

    if (!transform)
        return;
    const float width = movement ? movement->colliderWidth : 32.f;
    const float height = movement ? movement->colliderHeight : 48.f;

    const sf::FloatRect bounds(transform->position.x, transform->position.y, width, height);

    const auto collected = tilemap.collectPowerupIfOverlapping(bounds);
    if(collected) {
        applyPowerupPickup(*collected);

        powerupFlashTimer = powerupFlashDuration;
        score += powerupScoreValue;
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
        score += goalScoreValue;
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
    if (!invincible) {
        if (SpriteComponent* sprite = player->getComponent<SpriteComponent>()) {
            sprite->getSprite().setColor(sf::Color(255, 255, 255, 255));
        }
    }

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
                sprite->getSprite().setScale(currentXScale, 0.25f);

            }
            if (AnimationComponent* anim = entity->getComponent<AnimationComponent>()) {
                anim->paused = true;
            }
            playerPhysics->velocityY = -250.f;
            playerPhysics->onGround = false;

        }
        else {
            if (isPoweredState(currentPowerState)) {
                setPlayerPowerState(PlayerPowerState::Small);
                invincible = true;
                invincibilityTimer = invincibilityDuration;

            }
            else {
                loseLife();
            }
        }
    }

}
void EngineCore::resetLevelState() {
    levelComplete = false;
    goalMessageTimer = 0.f;
    collectedCoins = 0;
    coinBank = 0;
    playerDying = false;
    invincible = false;
    invincibilityTimer = 0.f;
    powerupFlashTimer = 0.f;
    tilemap.resetCollectibles();
    tilemap.resetPowerups();
    reservePowerup.reset();
    setPlayerPowerState(PlayerPowerState::Small);
    if (player) {
        if (MovementComponent* movement = player->getComponent<MovementComponent>()) {
            movement->enabled = true;
            movement->setCrouching(false);
        }
        if (PhysicsComponent* physics = player->getComponent<PhysicsComponent>()) {
            physics->enabled = true;
        }
        if (AnimationComponent* anim = player->getComponent<AnimationComponent>()) {
            anim->paused = false;
        }
    }
    respawnPlayer();

}
void EngineCore::updateInvincibility(float dt) {
    if (!invincible)
        return;
    invincibilityTimer = std::max(0.f, invincibilityTimer - dt);
    if (invincibilityTimer <= 0.f) {
        invincible = false;
        if (player) {
            if (SpriteComponent* sprite = player->getComponent<SpriteComponent>()) {
                sprite->getSprite().setColor(sf::Color(255, 255, 255, 255));
            }
        }
    }
    else if (player) {
        if (SpriteComponent* sprite = player->getComponent<SpriteComponent>()) {
            const float alpha = (static_cast<int>(invincibilityTimer * 10.f) % 2 == 0) ? 120.f : 255.f;
            sprite->getSprite().setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
        }
    }
}

void EngineCore::updatePowerupFlash(float dt) {
    if (powerupFlashTimer <= 0.f || !player) {
        return;
    }
    powerupFlashTimer = std::max(0.f, powerupFlashTimer - dt);

    if (SpriteComponent* sprite = player->getComponent<SpriteComponent>()) {
        const float t = powerupFlashTimer / std::max(powerupFlashDuration, 0.001f);
        const float pulse = 1.f + 0.1f * std::sin((1.f - t) * 12.f);
        const sf::Vector2f currentScale = sprite->getSprite().getScale();
        const float baseX = (currentScale.x < 0.f) ? -1.f : 1.f;
        float baseY = isPoweredState(currentPowerState) ? 1.33f : 1.f;
        if (MovementComponent* movement = player->getComponent<MovementComponent>()) {
            baseY = movement->baseScaleY * (movement->isCrouching ? movement->crouchScale : 1.f);
        }
        sprite->getSprite().setScale(
            baseX * pulse,
            baseY * pulse);
        const sf::Uint8 alpha = static_cast<sf::Uint8>(200 + 55 * t);
        sprite->getSprite().setColor(sf::Color(255, 255, 200, alpha));
    }
    if (powerupFlashTimer <= 0.f) {
        if (SpriteComponent* sprite = player->getComponent<SpriteComponent>()) {
            sprite->getSprite().setColor(sf::Color(255, 255, 255, 255));
            const sf::Vector2f currentScale = sprite->getSprite().getScale();
            const float baseX = (currentScale.x < 0.f) ? -1.f : 1.f;
            float baseY = isPoweredState(currentPowerState) ? 1.33f : 1.f;
            if (MovementComponent* movement = player->getComponent<MovementComponent>()) {
                baseY = movement->baseScaleY * (movement->isCrouching ? movement->crouchScale : 1.f);
            }
            sprite->getSprite().setScale(baseX, baseY);
        }
    }
}
void EngineCore::loseLife() {
    if (invincible || gameOver || playerDying)
        return;
    lives = std::max(0, lives - 1);
    invincible = true;
    invincibilityTimer = invincibilityDuration;

    if (lives <= 0) {
        gameOver = true;
        invincible = false;
        invincibilityTimer = 0.f;
        if (player) {
            if (SpriteComponent* sprite = player->getComponent<SpriteComponent>()) {
                sprite->getSprite().setColor(sf::Color(255, 255, 255, 255));

            }
        }
        return;

    }
    startPlayerDeath();
}
void EngineCore::resetGameState() {
    score = 0;
    lives = 3;
    gameOver = false;
    paused = false;
    playerDying = false;
    invincible = false;
    invincibilityTimer = 0.f;
    powerupFlashTimer = 0.f;
    currentLevelIndex = 0;
    maxUnlockedLevelIndex = 0;
    selectedLevelIndex = 0;
    reservePowerup.reset();
    saveProgress();
    loadLevel(currentLevelIndex); 
}

void EngineCore::setPlayerPowerState(PlayerPowerState powerState) {
    if (!player)
        return;

    if (currentPowerState == powerState)
        return;

    TransformComponent* transform = player->getComponent<TransformComponent>();
    MovementComponent* movement = player->getComponent<MovementComponent>();
    PhysicsComponent* physics = player->getComponent<PhysicsComponent>();
    SpriteComponent* sprite = player->getComponent<SpriteComponent>();

    const float oldHeight = movement ? movement->colliderHeight : smallColliderHeight;
    const float powered = isPoweredState(powerState);
    const float baseHeight = powered ? bigColliderHeight : smallColliderHeight;
    float newHeight = baseHeight;

    if (movement) {
        movement->standingColliderHeight = baseHeight;
        movement->crouchColliderHeight = baseHeight * movement->crouchHeightRatio;
        newHeight = movement->isCrouching ? movement->crouchColliderHeight : movement->standingColliderHeight;
    }

    const float delta = newHeight - oldHeight;

    if (transform && std::abs(delta) > 0.10f) {
        transform->position.y -= delta;

    }
    if (movement) {
        movement->colliderHeight = newHeight;
        movement->standingColliderHeight = baseHeight;
        movement->setBaseScaleY(powered ? 1.33f : 1.f);

    }
    if (physics) {
        physics->colliderHeight = newHeight;

    }
    if (sprite) {
        const float xScale = sprite->getSprite().getScale().x;
        float yScale = powered ? 1.33f : 1.f;
        if (movement) {
            yScale = movement->baseScaleY * (movement->isCrouching ? movement->crouchScale : 1.f);
        }
        sprite->getSprite().setScale(xScale, yScale);

    }
    currentPowerState = powerState;
    applyPowerupMovementModifiers();

}

void EngineCore::handleReserveActivation() {
    if (!reservePowerup) {
        return;
    }
    setPlayerPowerState(*reservePowerup);
    reservePowerup.reset();
    powerupFlashTimer = powerupFlashDuration;
}

void EngineCore::applyPowerupPickup(Tilemap::PowerupType powerupType) {
    const PlayerPowerState newState = toPlayerPowerState(powerupType);
    if (isPoweredState(currentPowerState)) {
        reservePowerup = newState;
        return;
    }
    setPlayerPowerState(newState);
}

void EngineCore::applyPowerupMovementModifiers() {
    if (!player) {
        return;
    }
    MovementComponent* movement = player->getComponent<MovementComponent>();
    PhysicsComponent* physics = player->getComponent<PhysicsComponent>();

    if (movement) {
        movement->walkSpeed = baseWalkSpeed;
        movement->runSpeed = baseRunSpeed;
        movement->crouchSpeed = baseCrouchSpeed;
    }
    if (physics) {
        physics->gravity = baseGravity;
        physics->jumpStrength = baseJumpStrength;
    }

    switch (currentPowerState) {
    case PlayerPowerState::SuperLeaf:
    case PlayerPowerState::TanookiSuit:
        if (movement) {
            movement->runSpeed = baseRunSpeed * 1.1f;
        }
        if (physics) {
            physics->gravity = baseGravity * 0.85f;
            physics->jumpStrength = baseJumpStrength * 1.12f;
        }
        break;
    case PlayerPowerState::FrogSuit:
        if (movement) {
            movement->walkSpeed = baseWalkSpeed * 0.8f;
            movement->runSpeed = baseRunSpeed * 0.8f;
        }
        if (physics) {
            physics->gravity = baseGravity * 0.75f;
            physics->jumpStrength = baseJumpStrength * 1.05f;
        }
        break;
    case PlayerPowerState::HammerSuit:
        if (movement) {
            movement->runSpeed = baseRunSpeed * 0.9f;
        }
        if (physics) {
            physics->gravity = baseGravity * 1.1f;
            physics->jumpStrength = baseJumpStrength * 0.9f;
        }
        break;
    default:
        break;
    }
}

void EngineCore::applyGlidePhysics() {
    if (!player) {
        return;
    }
    if (currentPowerState != PlayerPowerState::SuperLeaf
        && currentPowerState != PlayerPowerState::TanookiSuit) {
        return;
    }
    PhysicsComponent* physics = player->getComponent<PhysicsComponent>();
    if (!physics || physics->onGround) {
        return;
    }
    const bool glideHeld = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
    if (!glideHeld || physics->velocityY <= 0.f) {
        return;
    }
    const float glideFallSpeed = 220.f;
    physics->velocityY = std::min(physics->velocityY, glideFallSpeed);
}

EngineCore::PlayerPowerState EngineCore::toPlayerPowerState(Tilemap::PowerupType powerupType) {
    switch (powerupType) {
    case Tilemap::PowerupType::FireFlower:
        return PlayerPowerState::FireFlower;
    case Tilemap::PowerupType::SuperLeaf:
        return PlayerPowerState::SuperLeaf;
    case Tilemap::PowerupType::TanookiSuit:
        return PlayerPowerState::TanookiSuit;
    case Tilemap::PowerupType::HammerSuit:
        return PlayerPowerState::HammerSuit;
    case Tilemap::PowerupType::FrogSuit:
        return PlayerPowerState::FrogSuit;
    case Tilemap::PowerupType::SuperMushroom:
    default:
        return PlayerPowerState::SuperMushroom;
    }
}

std::string EngineCore::toPowerupLabel(PlayerPowerState powerState) {
    switch (powerState) {
    case PlayerPowerState::Small:
        return "Small";
    case PlayerPowerState::SuperMushroom:
        return "Super";
    case PlayerPowerState::FireFlower:
        return "Fire Flower";
    case PlayerPowerState::SuperLeaf:
        return "Super Leaf";
    case PlayerPowerState::TanookiSuit:
        return "Tanooki Suit";
    case PlayerPowerState::HammerSuit:
        return "Hammer Suit";
    case PlayerPowerState::FrogSuit:
        return "Frog Suit";
    default:
        return "Unknown";
    }
}

bool EngineCore::isPoweredState(PlayerPowerState powerState) {
    return powerState != PlayerPowerState::Small;
}

