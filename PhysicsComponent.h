#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include "Tilemap.h"
#include <SFML/Window/Keyboard.hpp>

class PhysicsComponent : public Component {
public:
    TransformComponent* transform;
    Tilemap* tilemap;

    float velocityY = 0.f;
    float gravity = 900.f;
    float jumpStrength = -500.f;
    bool onGround = false;
    float colliderWidth = 32.f;
    float colliderHeight = 48.f;


    PhysicsComponent(TransformComponent* transform, Tilemap* tilemap,
        float colliderWidth = 32.f, float colliderHeight = 48.f)
        : transform(transform), tilemap(tilemap), colliderWidth(colliderWidth), colliderHeight(colliderHeight) {

    }

    void update(float dt) override {
        if (!transform || !tilemap) return;

        // Jump
        if (onGround && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            velocityY = jumpStrength;
            onGround = false;
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
    }
};