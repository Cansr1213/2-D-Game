#pragma once
#include "Window.h"
#include "Scene.h"
#include "Tilemap.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


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
    sf::Text powerText;
    sf::Text levelText;
    sf::Text startMenuTitleText;
    sf::Text startMenuPromptText;
    sf::Text beginText;
    sf::RenderTexture pixelateTexture;
    sf::Sprite pixelateSprite;
    sf::Vector2u pixelateTextureSize{ 0, 0 };
    sf::Music backgroundMusic;
    float goalMessageTimer = 0.f;
    const float goalMessageDuration = 2.5f;
    bool invincible = false;
	float invincibilityTimer = 0.f;
	const float invincibilityDuration = 1.5f;
    bool poweredUp = false;
    const int coinScoreValue = 100;
    const int goalScoreValue = 500;
    const int powerupScoreValue = 1000;
    const float smallColliderHeight = 48.f;
    const float bigColliderHeight = 64.f;
    float powerupFlashTimer = 0.f;
    float powerupFlashDuration = 0.35f;
    int coinBank = 0;
    int currentWorld = 1;
    int currentLevel = 1;
    bool inStartMenu = true;
    bool startTransition = false;
    float startTransitionTimer = 0.f;
    const float startTransitionDuration = 1.2f;
    const float beginTextDuration = 0.5f;







    void processEvents();
    void update(float dt);
    void render();
    void renderScene(sf::RenderTarget& target);
    void renderPixelatedScene();

    void clampCameraToLevel();
    void respawnPlayer();
    void resetPlayerIfFallen();
    void handleCollectibles();
    void handlePowerups();
    void checkGoalReached();
    void handleEnemyCollisions();
    void updateInvincibility(float dt);
    void updatePowerupFlash(float dt);
    void loseLife();
    void resetLevelState();
    void resetGameState();
    void setPlayerPowerState(bool powered);



};
