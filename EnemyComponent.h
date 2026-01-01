#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include "Entity.h"
#include "Tilemap.h"
#include "SpriteComponent.h"
#include "PhysicsComponent.h"

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
                if (deathTimer <= 0.f) {
                    entity->destroy();

                }
            }
            return;
        }

        float dx = direction * speed * dt;
        float newX = transform->position.x + dx;

        float feetY = transform->position.y + colliderHeight - 1.f;
        int tileY = static_cast<int>(feetY) / tilemap->tileSize;
        int tileXLeft = static_cast<int>(newX) / tilemap->tileSize;
        int tileXRight = static_cast<int>(newX + colliderWidth - 1.f) / tilemap->tileSize;

        bool hitWall = false;

        if (dx < 0.f) {
            if (tilemap->isSolid(tileXLeft, tileY)) {
                transform->position.x = (tileXLeft + 1) * tilemap->tileSize;
                hitWall = true;
            }
            else {
                transform->position.x = newX;
            }
        }
        else if (dx > 0.f) {
            if (tilemap->isSolid(tileXRight, tileY)) {
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
        if (hitWall && onGround) {
            direction *= -1;
            turned = true;
        }

        if (onGround && !turned) {
            int tileBelow = tileY + 1;
            int tileAheadX = direction < 0
                ? static_cast<int>((transform->position.x - 1.f) / tilemap->tileSize)
                : static_cast<int>((transform->position.x + colliderWidth) / tilemap->tileSize);

            if (!tilemap->isSolid(tileAheadX, tileBelow)) {
                direction *= -1;
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