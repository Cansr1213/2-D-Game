#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include "Tilemap.h"
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>

class PhysicsComponent : public Component {
public:
    TransformComponent* transform;
    Tilemap* tilemap;

    bool enabled = true;
    float velocityY = 0.f;
    float gravity = 900.f;
    float jumpStrength = -500.f;
    bool onGround = false;
    bool allowJumpInput = true;
    float coyoteTime = 0.1f;
    float jumpBufferTime = 0.1f;
    float coyoteTimer = 0.f;
    float jumpBufferTimer = 0.f;
    float jumpCutMultiplier = 0.45f;
    bool jumpHeldLastFrame = false;
    float colliderWidth = 32.f;
    float colliderHeight = 48.f;


    PhysicsComponent(TransformComponent* transform, Tilemap* tilemap,
        float colliderWidth = 32.f, float colliderHeight = 48.f, bool allowJumpInput = true)
        : transform(transform), tilemap(tilemap), colliderWidth(colliderWidth), colliderHeight(colliderHeight), allowJumpInput(allowJumpInput)


    {

    }

    void update(float dt) override {
        if (!enabled || !transform || !tilemap) return;

        const bool jumpPressed = allowJumpInput && sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
        if (jumpPressed && !jumpHeldLastFrame) {
            jumpBufferTimer = jumpBufferTime;

        
        }
        else if (jumpBufferTimer > 0.f) {
            jumpBufferTimer = std::max(0.f, jumpBufferTime - dt);

        }
        if (onGround) {
            coyoteTimer = coyoteTime;
        }
        else if (coyoteTimer > 0.f) {
            coyoteTimer = std::max(0.f, coyoteTimer - dt);

        }
        if (jumpBufferTimer > 0.f && coyoteTimer > 0.f) {
            velocityY = jumpStrength;
            onGround = false;
            jumpBufferTimer = 0.f;
            coyoteTimer = 0.f;
        }
        if (!jumpPressed && jumpHeldLastFrame && velocityY < 0.f) {
            velocityY *= jumpCutMultiplier;
        }
        // Apply gravity
        velocityY += gravity * dt;

        const float newY = transform->position.y + velocityY * dt;

        const int tileXLeft = static_cast<int>(transform->position.x) / tilemap->tileSize;
        const int tileXRight = static_cast<int>(transform->position.x + colliderWidth - 1.f) / tilemap->tileSize;
        if (velocityY >= 0.f) {

            const float feetY = newY + colliderHeight - 1.f;
            const int tileY = static_cast<int>(feetY) / tilemap->tileSize;

            bool collided = false;
            for (int x = tileXLeft; x <= tileXRight; ++x) {
                if (tilemap->isSolid(x, tileY)) {
                    transform->position.y = tileY * tilemap->tileSize - colliderHeight;
                    velocityY = 0.f;
                    onGround = true;
                    collided = true;
                    break;

                }
            }
            if (!collided) {
                transform->position.y = newY;
                onGround = false;

            }



        }
        else {
            const float headY = newY;
            const int tileY = static_cast<int>(headY) / tilemap->tileSize;

            bool collided = false;
            for (int x = tileXLeft; x <= tileXRight; ++x) {
                if (tilemap->isSolid(x, tileY)) {
                    transform->position.y = (tileY + 1) * tilemap->tileSize;
                    velocityY = 0.f;
                    collided = true;
                    break;

                }
            }
            if (!collided) {
                transform->position.y = newY;

            }
            onGround = false;

        }
        jumpHeldLastFrame = jumpPressed;
    }
};
