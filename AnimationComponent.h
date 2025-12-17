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
    int idleRow = 0;   // row 2
    int walkRow = 0;  // row 3

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
        if (const sf::Texture* tex = spriteComp->getSprite().getTexture()) {
            
            auto size = tex->getSize();
            if (frameCount > 0) {
                frameWidth = static_cast<int>(size.x) / frameCount;
            }
            if (frameHeight <= 0) {
                frameHeight = static_cast<int>(size.y);


            }
        }
        idleRow = 0;


        if (const sf::Texture* tex = spriteComp->getSprite().getTexture()) {

            const auto textSize = tex->getSize();
            const int rows = static_cast<int>(textSize.y) / frameHeight;
            const int cols = (frameWidth > 0) ? static_cast<int>(textSize.x) / frameWidth : 0;

            walkRow = idleRow;

            if (rows > 0) {
                const sf::Image image = tex->copyToImage();
                float bestScore = -1.f;
                int bestRow = idleRow;


                for (int r = 1; r < rows; ++r) {
                    const int yStart = r * frameHeight;
                    unsigned long long alphaSum = 0;
                    unsigned long long weightSum = 0;
                    unsigned long long bottomAlpha = 0;

                    for (int y = 0; y < frameHeight; ++y) {
                        const int globalY = yStart + y;
                        for (unsigned int x = 0; x < textSize.x; ++x) {
                            const sf::Color px = image.getPixel(x, globalY);


                            alphaSum += px.a;
                            weightSum += static_cast<unsigned long long>(px.a) * y;

                            if (y >= (frameHeight * 2) / 3) {
                                bottomAlpha += px.a;
                            }

                        }
                    }
                    if (alphaSum == 0) continue;

                    const float center = static_cast<float>(weightSum) / static_cast<float>(alphaSum);
                    const float bottomRatio = static_cast<float>(bottomAlpha) / static_cast<float>(alphaSum);
                    const float score = center + bottomRatio * frameHeight;

                    if (score > bestScore) {
                        bestScore = score;
                        bestRow = r * frameHeight;

                    }
                }
                walkRow = bestRow;

            }
        }
       
   

        spriteComp->frameWidth = frameWidth;
        spriteComp->frameHeight = frameHeight;
        spriteComp->getSprite().setOrigin(frameWidth / 2.f, frameHeight / 2.f);

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
