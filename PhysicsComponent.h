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
    float jumpStrength = -400.f;
    bool onGround = false;

    PhysicsComponent(TransformComponent* transform, Tilemap* tilemap)
        : transform(transform), tilemap(tilemap) {
    }

    void update(float dt) override {
        if (!transform) return;

        // Jump
        if (onGround && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            velocityY = jumpStrength;
            onGround = false;
        }

        // Apply gravity
        velocityY += gravity * dt;

        // Predict new vertical position
        float newY = transform->position.y + velocityY * dt;

        // Convert new position to tile coords
        int tileX = transform->position.x / tilemap->tileSize;
        int tileY = newY / tilemap->tileSize;

        // COLLISION: check if the tile below is solid
        if (tilemap->isSolid(tileX, tileY)) {
            // Snap player on top of tile
            transform->position.y = tileY * tilemap->tileSize;
            velocityY = 0.f;
            onGround = true;
        }
        else {
            transform->position.y = newY;
            onGround = false;
        }
    }
};