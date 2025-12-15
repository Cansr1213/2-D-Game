#pragma once
#include "Component.h"
#include "SpriteComponent.h"

enum class AnimState {
    Idle,
    Walk
};

class AnimationComponent : public Component {
public:
    SpriteComponent* spriteComp;

    int frameWidth;
    int frameHeight;

    float frameTime;
    float currentTime = 0.f;

    // Frame ranges
    int idleStart = 0;
    int idleEnd = 5;

    int walkStart = 0;
    int walkEnd = 5;

    // Row offsets (Y position in spritesheet)
    int idleRow = 64;   // row 2
    int walkRow = 128;  // row 3

    AnimState state = AnimState::Idle;
    int currentFrame = 0;

    AnimationComponent(SpriteComponent* spriteComp,
        int frameWidth,
        int frameHeight,
        int frameCount,
        float frameTime)
        : spriteComp(spriteComp),
        frameWidth(frameWidth),
        frameHeight(frameHeight),
        frameTime(frameTime)
    {
        spriteComp->frameWidth = frameWidth;
        spriteComp->frameHeight = frameHeight;

        // Start idle
        spriteComp->getSprite().setTextureRect(
            sf::IntRect(0, idleRow, frameWidth, frameHeight)
        );
    }

    void setState(AnimState newState)
    {
        if (state == newState) return;

        state = newState;
        currentTime = 0.f;
        currentFrame = (state == AnimState::Idle) ? idleStart : walkStart;

        int row = (state == AnimState::Idle) ? idleRow : walkRow;

        spriteComp->getSprite().setTextureRect(
            sf::IntRect(currentFrame * frameWidth, row, frameWidth, frameHeight)
        );
    }

    void update(float dt) override {
        currentTime += dt;
        if (currentTime < frameTime) return;
        currentTime = 0.f;

        int start = (state == AnimState::Idle) ? idleStart : walkStart;
        int end = (state == AnimState::Idle) ? idleEnd : walkEnd;
        int row = (state == AnimState::Idle) ? idleRow : walkRow;

        currentFrame++;
        if (currentFrame > end)
            currentFrame = start;

        spriteComp->getSprite().setTextureRect(
            sf::IntRect(currentFrame * frameWidth, row, frameWidth, frameHeight)
        );
    }
};
