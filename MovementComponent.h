#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include "Tilemap.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include "PhysicsComponent.h"
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <cmath>

class MovementComponent : public Component {
public:
    TransformComponent* transform;
    Tilemap* tilemap;

    float walkSpeed = 180.f;
    float runSpeed = 260.f;
    float acceleration = 1400.f;
    float deacceleration = 2000.f;
    float airAcceleration = 800.f;
    float airDeacceleration = 700.f;
    float skidDeceleration = 2400.f;
    float airSpeedMultiplier = 0.9f;

    
    
    
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
        PhysicsComponent* physics = entity->getComponent<PhysicsComponent>();
        const bool grounded = physics ? physics->onGround : true;

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
        const float effectiveMaxSpeed = grounded ? maxSpeed : maxSpeed * airAcceleration;


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
            targetVelocity = std::clamp(targetVelocity, -effectiveMaxSpeed, effectiveMaxSpeed);
            const float delta = targetVelocity - velocityX;
            const float accelRate = grounded ? acceleration : airAcceleration;
            const float accel = accelRate * dt;

            if (std::abs(delta) <= accel) {
                velocityX = targetVelocity;
            }
            else {
                velocityX += (delta > 0.f ? accel : -accel);
            }
        }
        else {
            const float decelRate = grounded ? deacceleration : airDeacceleration;
            const float decel = decelRate * dt;
            if (std::abs(velocityX) <= decel) {
                velocityX = 0.f;
            }
            else {
                velocityX += (velocityX > 0.f ? -decel : decel);
            }
        }
        if (grounded && targetVelocity != 0.f && (targetVelocity * velocityX) < 0.f) {
            const float skid = skidDeceleration * dt;
            if (std::abs(velocityX) <= skid) {
                velocityX = 0.f;

            }
            else {
                velocityX += (velocityX > 0.f ? -skid : skid);
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
        const float topY = transform->position.y + 1.f;
        const float bottomY = transform->position.y + colliderHeight - 1.f;
        const int tileYTop = static_cast<int>(topY) / tilemap->tileSize;
        const int tileYBottom = static_cast<int>(bottomY) / tilemap->tileSize;



        int tileXLeft =
            static_cast<int>(newX) / tilemap->tileSize;

        int tileXRight =
            static_cast<int>(newX + colliderWidth - 1.f) / tilemap->tileSize;
        auto isBlocked = [&](int tileX) {
            for (int tileY = tileYTop; tileY <= tileYBottom; ++tileY) {
                if (tilemap->isSolid(tileX, tileY)) {
                    return true;
                }
            }
            return false;
            };

   

        // Moving left
        if (velocityX < 0.f) {
            if (isBlocked(tileXLeft)) {
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
            if (isBlocked(tileXRight)) {
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
