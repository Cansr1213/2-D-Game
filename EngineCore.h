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
    int score = 0;
    int lives = 3;
    bool levelComplete = false;
    bool resetHeld = false;
    bool pausedHeld = false;
    bool paused = false;
    bool gameOver = false;
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
    sf::Text scoreText;
    sf::Text livesText;
    sf::Text pauseText;
    sf::Text goalText;
    sf::Text gameOverText;
    sf::Text controlsText;
    float goalMessageTimer = 0.f;
    const float goalMessageDuration = 2.5f;
    bool invincible = false;
	float invincibilityTimer = 0.f;
	const float invincibilityDuration = 1.5f;
    const int coinScoreValue = 100;
    const int goalScoreValue = 500;






    void processEvents();
    void update(float dt);
    void render();

    void clampCameraToLevel();
    void respawnPlayer();
    void resetPlayerIfFallen();
    void handleCollectibles();
    void checkGoalReached();
    void handleEnemyCollisions();
    void updateInvincibility(float dt);
    void loseLife();
    void resetLevelState();
    void resetGameState();


};
