#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include "Tilemap.h"
#include "Scene.h"
#include "EnemyComponent.h"
#include "PhysicsComponent.h"
#include "SpriteComponent.h"
#include "AnimationComponent.h"
#include <algorithm>

class ProjectileComponent : public Component {
public:
    ProjectileComponent(TransformComponent* transform,
        Tilemap* tilemap,
        Scene* scene,
        float velocityX,
        float velocityY,
        float colliderWidth,
        float colliderHeight,
        float lifetime,
        float gravity = 0.f)
        : transform(transform),
        tilemap(tilemap),
        scene(scene),
        velocityX(velocityX),
        velocityY(velocityY),
        colliderWidth(colliderWidth),
        colliderHeight(colliderHeight),
        lifetime(lifetime),
        gravity(gravity) {
    }

    void update(float dt) override {
        if (!transform || !tilemap) {
            return;
        }
        lifetime = std::max(0.f, lifetime - dt);
        if (lifetime <= 0.f) {
            entity->destroy();
            return;
        }

        if (gravity != 0.f) {
            velocityY += gravity * dt;
        }

        sf::Vector2f nextPosition = {
            transform->position.x + velocityX * dt,
            transform->position.y + velocityY * dt
        };

        if (collidesWithSolid(nextPosition)) {
            entity->destroy();
            return;
        }

        transform->position = nextPosition;

        if (scene) {
            if (checkEnemyHit()) {
                entity->destroy();
                return;
            }
        }

        const float levelWidth = static_cast<float>(tilemap->getPixelWidth());
        const float levelHeight = static_cast<float>(tilemap->getPixelHeight());
        if (nextPosition.x < -colliderWidth
            || nextPosition.x > levelWidth + colliderWidth
            || nextPosition.y < -colliderHeight
            || nextPosition.y > levelHeight + colliderHeight) {
            entity->destroy();
        }
    }

private:
    TransformComponent* transform = nullptr;
    Tilemap* tilemap = nullptr;
    Scene* scene = nullptr;
    float velocityX = 0.f;
    float velocityY = 0.f;
    float colliderWidth = 16.f;
    float colliderHeight = 16.f;
    float lifetime = 0.f;
    float gravity = 0.f;

    bool collidesWithSolid(const sf::Vector2f& position) const {
        const float leftX = position.x;
        const float rightX = position.x + colliderWidth - 1.f;
        const float topY = position.y;
        const float bottomY = position.y + colliderHeight - 1.f;

        const int tileXLeft = static_cast<int>(leftX) / tilemap->tileSize;
        const int tileXRight = static_cast<int>(rightX) / tilemap->tileSize;
        const int tileYTop = static_cast<int>(topY) / tilemap->tileSize;
        const int tileYBottom = static_cast<int>(bottomY) / tilemap->tileSize;

        for (int y = tileYTop; y <= tileYBottom; ++y) {
            for (int x = tileXLeft; x <= tileXRight; ++x) {
                if (tilemap->isSolid(x, y)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool checkEnemyHit() const {
        const sf::FloatRect bounds(
            transform->position.x,
            transform->position.y,
            colliderWidth,
            colliderHeight);

        for (auto& other : scene->getEntities()) {
            if (!other || other.get() == entity) {
                continue;
            }
            EnemyComponent* enemy = other->getComponent<EnemyComponent>();
            if (!enemy || !enemy->alive) {
                continue;
            }
            TransformComponent* enemyTransform = other->getComponent<TransformComponent>();
            if (!enemyTransform) {
                continue;
            }
            const sf::FloatRect enemyBounds(
                enemyTransform->position.x,
                enemyTransform->position.y,
                enemy->colliderWidth,
                enemy->colliderHeight);
            if (!bounds.intersects(enemyBounds)) {
                continue;
            }

            enemy->alive = false;
            enemy->deathTimer = enemy->deathDelay;
            if (PhysicsComponent* enemyPhysics = other->getComponent<PhysicsComponent>()) {
                enemyPhysics->velocityY = 0.f;
                enemyPhysics->onGround = true;
            }
            if (SpriteComponent* sprite = other->getComponent<SpriteComponent>()) {
                const float currentXScale = sprite->getSprite().getScale().x;
                sprite->getSprite().setScale(currentXScale, 0.25f);
            }
            if (AnimationComponent* anim = other->getComponent<AnimationComponent>()) {
                anim->paused = true;
            }
            return true;
        }
        return false;
    }
};