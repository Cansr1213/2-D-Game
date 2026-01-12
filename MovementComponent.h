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
    bool enabled = true;
    float walkSpeed = 180.f;
    float runSpeed = 260.f;
    float crouchSpeed = 90.f;
    float acceleration = 1400.f;
    float deacceleration = 2000.f;
    float airAcceleration = 800.f;
    float airDeacceleration = 700.f;
    float skidDeceleration = 2400.f;
    float airSpeedMultiplier = 0.9f;
    float crouchHeightRatio = 0.65f;
    float crouchScale = 0.7;



    
    
    
    float velocityX = 0.f;

    float colliderWidth = 32.f;
    float colliderHeight = 48.f;
    float standingColliderHeight = 48.f;
    float crouchColliderHeight = 31.2f;
    float baseScaleY = 1.f;
    float isCrouching = false;

    MovementComponent(TransformComponent* transform, Tilemap* tilemap)
        : transform(transform), tilemap(tilemap) {
        standingColliderHeight = colliderHeight;
        crouchColliderHeight = colliderHeight * crouchHeightRatio;
    }

    void update(float dt) override {
        if (!enabled || !transform || !tilemap) return;

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

        const bool crouchPressed =
            sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Down);

        const bool running =
            sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::RShift);

        if (grounded && crouchPressed) {
            setCrouching(true);
        }
        else if (!crouchPressed && grounded && isCrouching) {
            if (canStandUp(standingColliderHeight)) {
                setCrouching(false);
            }
        }
        if (physics) {
            physics->allowJumpInput = !isCrouching;
        }

        const float maxSpeed = isCrouching ? crouchSpeed : (running ? runSpeed : walkSpeed);
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
            if (isCrouching) {
                anim->setState(AnimState::Idle);
            }
            else if (std::abs(velocityX) <= 1.f){
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
    void setBaseScaleY(float newBaseScaleY) {
        baseScaleY = newBaseScaleY;
        if (SpriteComponent* sprite = entity ? entity->getComponent<SpriteComponent>() : nullptr) {
            applyScale(sprite);
        }
    }

    void setStandingHeight(float newStandingHeight) {
        if (!transform) {
            return;
        }
        standingColliderHeight = newStandingHeight;
        crouchColliderHeight = standingColliderHeight * crouchHeightRatio;
        float desiredHeight = isCrouching ? crouchColliderHeight : standingColliderHeight;
        const float delta = desiredHeight - colliderHeight;
        if (std::abs(delta) > 0.01f) {
            transform->position.y -= delta;
        }
        colliderHeight = desiredHeight;
        if (PhysicsComponent* physics = entity ? entity->getComponent<PhysicsComponent>() : nullptr) {
            physics->colliderHeight = desiredHeight;
        }
    }

    void setCrouching(bool crouch) {
        if (!transform) {
            return;
        }
        if (isCrouching == crouch) return;
        isCrouching = crouch;
        const float desiredHeight = crouch ? crouchColliderHeight : standingColliderHeight;
        const float delta = desiredHeight - colliderHeight;
        if (std::abs(delta) > 0.01f) {
            transform->position.y -= delta;
        }
        colliderHeight = desiredHeight;
        if (PhysicsComponent* physics = entity ? entity->getComponent<PhysicsComponent>() : nullptr) {
            physics->colliderHeight = desiredHeight;
        }
        if (SpriteComponent* sprite = entity ? entity->getComponent<SpriteComponent>() : nullptr) {
            applyScale(sprite);
        }
    }

private:
    void applyScale(SpriteComponent* sprite) {
        const sf::Vector2f currentScale = sprite->getSprite().getScale();
        const float absX = std::abs(currentScale.x) > 0.f ? std::abs(currentScale.x) : 1.f;
        const float baseX = (currentScale.x < 0.f) ? -absX : absX;
        const float scaleY = baseScaleY * (isCrouching ? crouchScale : 1.f);
        sprite->getSprite().setScale(baseX, scaleY);
    }

    bool canStandUp(float desiredHeight) {
        if (!tilemap || !transform) {
            return true;
        }
        const float delta = desiredHeight - colliderHeight;
        if (delta <= 0.f) {
            return true;
        }
        const float newTopY = transform->position.y - delta;
        const float leftX = transform->position.x;
        const float rightX = transform->position.x + colliderWidth - 1.f;
        const int tileXLeft = static_cast<int>(leftX) / tilemap->tileSize;
        const int tileXRight = static_cast<int>(rightX) / tilemap->tileSize;
        const int tileYTop = static_cast<int>(newTopY) / tilemap->tileSize;
        const int tileYBottom = static_cast<int>(newTopY + desiredHeight - 1.f) / tilemap->tileSize;

        for (int x = tileXLeft; x <= tileXRight; ++x) {
            for (int y = tileYTop; y <= tileYBottom; ++y) {
                if (tilemap->isSolid(x, y)) {
                    return false;
                }
            }
        }
        return true;
    }
};
