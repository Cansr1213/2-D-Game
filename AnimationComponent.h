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
    bool paused = false;

    int frameWidth;
    int frameHeight;
    int frameCount;
    int baseFrameWidth;
    int baseFrameHeight;
    int desiredFrameWidth;
    int desiredFrameHeight;
    float textureScaleX = 1.f;
    float textureScaleY = 1.f;


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
        frameCount(frameCount),
        baseFrameWidth(frameWidth),
        baseFrameHeight(frameHeight),
        desiredFrameWidth(0),
        desiredFrameHeight(0),
        frameTime(frameTime)
    {

        configureFromTexture(true);
    }
    void refreshFromTexture() {
        configureFromTexture(true);
   
    }
    float getTextureScaleX() const {
        return textureScaleX;
    }
    float getTextureScaleY() const {
        return textureScaleY;
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
        if (paused) {
            return;
        }
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
    private: 
        void configureFromTexture(bool resetState) {
            const sf::Texture* tex = spriteComp ? spriteComp->getSprite().getTexture() : nullptr;
            if (!tex) {
                return;

            }
            frameWidth = baseFrameWidth;
            frameHeight = baseFrameHeight;

            const auto size = tex->getSize();
            if (frameCount > 0) {
                frameWidth = static_cast<int>(size.x) / frameCount;

                const int columns = (frameCount > 0) ? frameCount : (frameWidth > 0 ? static_cast<int>(size.x) / frameWidth : 0);
                const bool heightDivides = (frameHeight > 0) && (size.y % frameHeight == 0);
                if (!heightDivides && columns > 0 && size.y % columns == 0) {
                    frameHeight = static_cast<int>(size.y) / columns;

                }
                else if (frameHeight <= 0) {
                    frameHeight = static_cast<int>(size.y);
                }

            }

            idleRow = 0;

            const auto textSize = tex->getSize();
            const int rows = (frameHeight > 0) ? static_cast<int>(textSize.y) / frameHeight : 0;
            const int cols = (frameWidth > 0) ? static_cast<int>(textSize.x) / frameWidth : 0;

            walkRow = (rows > 1) ? frameHeight : idleRow;
            if (rows > 0 && cols > 0) {
                const sf::Image image = tex->copyToImage();
                float bestScore = -1.f;
                int bestRow = walkRow;

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

            spriteComp->frameWidth = frameWidth;
            spriteComp->frameHeight = frameHeight;
            spriteComp->getSprite().setOrigin(frameWidth / 2.f, frameHeight / 2.f);

            if (resetState) {
                state = AnimState::Idle;
                currentFrame = idleStart;
                currentTime = 0.f;
            }

            const int row = (state == AnimState::Idle) ? idleRow : walkRow;
            spriteComp->getSprite().setTextureRect(
                sf::IntRect(currentFrame * frameWidth, row, frameWidth, frameHeight)
            );
        }


        
};
