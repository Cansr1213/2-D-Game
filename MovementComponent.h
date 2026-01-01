#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include "Tilemap.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

class MovementComponent : public Component {
public:
    TransformComponent* transform;
    Tilemap* tilemap;

    float walkSpeed = 180.f;
    float runSpeed = 260.f;
    float acceleration = 1200.f;
    float deacceleration = 1800.f;

    float velocityX = 0.f;

    float colliderWidth = 32.f;
    float colliderHeight = 48.f;

    MovementComponent(TransformComponent* transform, Tilemap* tilemap)
        : transform(transform), tilemap(tilemap) {
    }

    void update(float dt) override {
        if (!transform || !tilemap) return;

        float targetVelocity = 0.f;

        // Get components
        AnimationComponent* anim = entity->getComponent<AnimationComponent>();
        SpriteComponent* sprite = entity->getComponent<SpriteComponent>();

        // Input
        const bool moveLeft =
            sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Left);

        const bool moveRight =
            sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Right);

        const bool running =
            sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

        const float maxSpeed = running ? runSpeed : walkSpeed;

        // Direction intent
        if (moveLeft) {
            targetVelocity -= maxSpeed;
            if (sprite) sprite->setFlipped(true);
        }

        if (moveRight) {
            targetVelocity += maxSpeed;
            if (sprite) sprite->setFlipped(false);
        }

        // Acceleration / deceleration
        if (targetVelocity != 0.f) {
            const float delta = targetVelocity - velocityX;
            const float accel = acceleration * dt;

            if (std::abs(delta) <= accel) {
                velocityX = targetVelocity;
            }
            else {
                velocityX += (delta > 0.f ? accel : -accel);
            }
        }
        else {
            const float decel = deacceleration * dt;
            if (std::abs(velocityX) <= decel) {
                velocityX = 0.f;
            }
            else {
                velocityX += (velocityX > 0.f ? -decel : decel);
            }
        }

        // Animation state
        if (anim) {
            if (std::abs(velocityX) <= 1.f) {
                anim->setState(AnimState::Idle);
            }
            else {
                anim->setState(AnimState::Walk);
            }
        }

        // No horizontal movement
        if (velocityX == 0.f)
            return;

        // Proposed new X position
        float newX = transform->position.x + velocityX * dt;

        // Collision check at feet level
        float feetY = transform->position.y + colliderHeight - 1.f;
        int tileY = static_cast<int>(feetY) / tilemap->tileSize;

        int tileXLeft =
            static_cast<int>(newX) / tilemap->tileSize;

        int tileXRight =
            static_cast<int>(newX + colliderWidth - 1.f) / tilemap->tileSize;

        // Moving left
        if (velocityX < 0.f) {
            if (tilemap->isSolid(tileXLeft, tileY)) {
                transform->position.x =
                    (tileXLeft + 1) * tilemap->tileSize;
                velocityX = 0.f;
            }
            else {
                transform->position.x = newX;
            }
        }
        // Moving right
        else if (velocityX > 0.f) {
            if (tilemap->isSolid(tileXRight, tileY)) {
                transform->position.x =
                    tileXRight * tilemap->tileSize - colliderWidth;
                velocityX = 0.f;
            }
            else {
                transform->position.x = newX;
            }
        }
    }
};
