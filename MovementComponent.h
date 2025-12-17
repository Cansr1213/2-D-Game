#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include "Tilemap.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include <SFML/Window/Keyboard.hpp>

class MovementComponent : public Component {
public:
    TransformComponent* transform;
    Tilemap* tilemap;

    float speed = 200.f;
    float colliderWidth = 32.f;
    float colliderHeight = 48.f;

    MovementComponent(TransformComponent* transform, Tilemap* tilemap)
        : transform(transform), tilemap(tilemap) {
    }

    void update(float dt) override {
        if (!transform || !tilemap) return;

        float dx = 0.f;

        // Get animation + sprite
        AnimationComponent* anim = entity->getComponent<AnimationComponent>();
        SpriteComponent* sprite = entity->getComponent<SpriteComponent>();


        // INPUT
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
        {
            dx -= speed * dt;

            // Flip sprite left
            if (sprite) sprite->setFlipped(true);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
        {
            dx += speed * dt;

            // Flip sprite Right
            if (sprite) sprite->setFlipped(false);
        }
        if (anim) {

            if (dx == 0.f) anim->setState(AnimState::Idle);
            else {
                anim->setState(AnimState::Walk);
            }
        }

        if (dx == 0.f)
            return;

        // Predict new X position
        float newX = transform->position.x + dx;

        // Collision checking at player's feet level
        float feetY = transform->position.y + colliderHeight - 1.f;

        int tileY = static_cast<int>(feetY) / tilemap->tileSize;
        int tileXLeft = static_cast<int>(newX) / tilemap->tileSize;
        int tileXRight = static_cast<int>(newX + colliderWidth - 1.f) / tilemap->tileSize;

        if (dx < 0.f) {
            // Moving left
            if (tilemap->isSolid(tileXLeft, tileY)) {
                transform->position.x = (tileXLeft + 1) * tilemap->tileSize;
            }
            else {
                transform->position.x = newX;
            }
        }
        else if (dx > 0.f) {
            // Moving right
            if (tilemap->isSolid(tileXRight, tileY)) {
                transform->position.x = tileXRight * tilemap->tileSize - colliderWidth;
            }
            else {
                transform->position.x = newX;
            }
        }
    }
};
