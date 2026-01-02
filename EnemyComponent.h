#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include "Entity.h"
#include "Tilemap.h"
#include "SpriteComponent.h"
#include "PhysicsComponent.h"
#include <algorithm>

enum class EnemyType {
    Goomba
};

class EnemyComponent : public Component {
public:
    EnemyType type = EnemyType::Goomba;
    float speed = 60.f;
    int direction = -1;
    bool alive = true;
    float deathTimer = 0.f;
    float deathDelay = 0.3f;
    float colliderWidth = 32.f;
    float colliderHeight = 32.f;
    float turnCooldown = 0.12f;
    float turnTimer = 0.f;


    EnemyComponent(TransformComponent* transform, Tilemap* tilemap,
        float colliderWidth = 32.f, float colliderHeight = 32.f)
        : transform(transform), tilemap(tilemap),
        colliderWidth(colliderWidth), colliderHeight(colliderHeight) {
    }

    void update(float dt) override {
        if (!transform || !tilemap) return;
        if (!alive) {
            if (deathTimer > 0.f) {
                deathTimer -= dt;
                if (SpriteComponent* sprite = entity->getComponent<SpriteComponent>()) {
                    const float t = std::clamp(deathTimer / std::max(deathDelay, 0.001f), 0.f, 1.f);
                    const float currentXScale = sprite->getSprite().getScale().x;
                    const float squash = 0.1f + 0.4f * t;
                    sprite->getSprite().setScale(currentXScale, squash);
                }
                if (deathTimer <= 0.f) {
                    entity->destroy();

                }
            }
            return;
        }
        if (turnTimer > 0.f) {
            turnTimer = std::max(0.f, turnTimer - dt);

        }

        float dx = direction * speed * dt;
        float newX = transform->position.x + dx;

        const float topY = transform->position.y + 1.f;
        const float bottomY = transform->position.y + colliderHeight - 1.f;
        const int tileYTop = static_cast<int>(topY) / tilemap->tileSize;
        const int tileYBottom = static_cast<int>(bottomY) / tilemap->tileSize;
        int tileXLeft = static_cast<int>(newX) / tilemap->tileSize;
        int tileXRight = static_cast<int>(newX + colliderWidth - 1.f) / tilemap->tileSize;

        bool hitWall = false;
        auto isBlocked = [&](int tileX) {
            for (int tileY = tileYTop; tileY <= tileYBottom; ++tileY) {
                if (tilemap->isSolid(tileX, tileY)) {
                    return true;
                }
            }
            return false;
            };


        if (dx < 0.f) {
            if (isBlocked(tileXLeft)) {
                transform->position.x = (tileXLeft + 1) * tilemap->tileSize;
                hitWall = true;
            }
            else {
                transform->position.x = newX;
            }
        }
        else if (dx > 0.f) {
            if (isBlocked(tileXLeft)) {
                transform->position.x = tileXRight * tilemap->tileSize - colliderWidth;
                hitWall = true;
            }
            else {
                transform->position.x = newX;
            }
        }

        bool onGround = true;
        if (PhysicsComponent* physics = entity->getComponent<PhysicsComponent>()) {
            onGround = physics->onGround;
        }

        bool turned = false;
        if (hitWall && onGround && turnTimer == 0.f) {
            direction *= -1;
            turned = true;
            turnTimer = turnCooldown;
        }

        if (onGround && !turned && turnTimer == 0.f) {
            const float aheadX = direction < 0.f
                ? transform->position.x - 1.f : transform->position.x + colliderHeight + 1.f;
            const int tileAheadX = static_cast<int>(aheadX) / tilemap->tileSize;
            const int tileBelow = static_cast<int>((bottomY + 1.f) / tilemap->tileSize);


            if (!tilemap->isSolid(tileAheadX, tileBelow)) {
                direction *= -1;
                turnTimer = turnCooldown;
            }
        }

        if (SpriteComponent* sprite = entity->getComponent<SpriteComponent>()) {
            sprite->setFlipped(direction < 0);
        }
    }

private:
    TransformComponent* transform;
    Tilemap* tilemap;
};