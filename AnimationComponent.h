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
        const sf::Texture* tex = spriteComp->getSprite().getTexture();

        if (tex) {

            const auto size = tex->getSize();
            if (frameCount > 0) {
                this->frameWidth = static_cast<int>(size.x) / frameCount;


                const int columns = (frameCount > 0) ? frameCount : (this->frameWidth > 0 ? static_cast<int>(size.x) / this->frameWidth : 0);
                const bool heightDivides = (this->frameHeight > 0) && (size.y % this->frameHeight == 0);
                if (!heightDivides && columns > 0 && size.y % columns == 0) {
                    this->frameHeight = static_cast<int>(size.y) / columns;
                }
                else if (this->frameHeight <= 0) {
                    this->frameHeight = static_cast<int>(size.y);
                }

            }
        }



        idleRow = 0;


        if (tex) {

            const auto textSize = tex->getSize();
            const int rows = (this->frameHeight > 0) ? static_cast<int>(textSize.y) / this->frameHeight : 0;
            const int cols = (this->frameWidth > 0) ? static_cast<int>(textSize.x) / this->frameWidth : 0;

            
            walkRow = (rows > 1) ? this->frameHeight : idleRow;
            if (rows > 0) {
                const sf::Image image = tex->copyToImage();
                float bestScore = -1.f;
                int bestRow = walkRow;


                for (int r = 1; r < rows; ++r) {
                    const int yStart = r * this->frameHeight;
                    unsigned long long alphaSum = 0;
                    unsigned long long weightSum = 0;
                    unsigned long long bottomAlpha = 0;

                    for (int y = 0; y < this->frameHeight; ++y) {
                        const int globalY = yStart + y;
                        for (unsigned int x = 0; x < textSize.x; ++x) {
                            const sf::Color px = image.getPixel(x, globalY);


                            alphaSum += px.a;
                            weightSum += static_cast<unsigned long long>(px.a) * y;

                            if (y >= (this->frameHeight * 2) / 3) {
                                bottomAlpha += px.a;
                            }

                        }
                    }
                    if (alphaSum == 0) continue;

                    const float center = static_cast<float>(weightSum) / static_cast<float>(alphaSum);
                    const float bottomRatio = static_cast<float>(bottomAlpha) / static_cast<float>(alphaSum);
                    const float score = center + bottomRatio * this->frameHeight;

                    if (score > bestScore) {
                        bestScore = score;
                        bestRow = r * this->frameHeight;

                    }
                }
                walkRow = bestRow;

            }
        }
       
   

        spriteComp->frameWidth = this->frameWidth;
        spriteComp->frameHeight = this->frameHeight;
        spriteComp->getSprite().setOrigin(this->frameWidth / 2.f, this->frameHeight / 2.f);

        // Start idle
        spriteComp->getSprite().setTextureRect(
            sf::IntRect(0, idleRow, this->frameWidth, this->frameHeight)
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
            sf::IntRect(currentFrame * this->frameWidth, row, this->frameWidth, this->frameHeight)
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
