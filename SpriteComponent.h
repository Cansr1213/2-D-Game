#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

class SpriteComponent : public Component {
public:
    int frameWidth = 32;
    int frameHeight = 32;
    bool flipped = false;
    bool visible = true;


    SpriteComponent(const std::string& textureFile, TransformComponent* transform)
        : transform(transform)
    {
        if (!texture.loadFromFile(textureFile)) {
            std::cout << "FAILED TO LOAD SPRITE\n";
        }

        sprite.setTexture(texture);


        // ✅ Center origin ONCE
        sprite.setOrigin(frameWidth / 2.f, frameHeight / 2.f);
    }

    void update(float dt) override {
        // ✅ Keep physics + rendering aligned
        sprite.setPosition(
            transform->position.x + frameWidth / 2.8f,
            transform->position.y + frameHeight / 2.8f);
    }

    void render(sf::RenderWindow& window) override {
        if (!visible) return;
        window.draw(sprite);
    }

    sf::Sprite& getSprite() { return sprite; }

    // ✅ Flip using scale ONLY
    void setFlipped(bool flip) {
        if (flipped == flip) return;
        flipped = flip;
        const sf::Vector2f currentScale = sprite.getScale();
        const float xScale = std::abs(currentScale.x) > 0.f ? std::abs(currentScale.x) : 1.f;
        const float yScale = currentScale.y == 0.f ? 1.f : currentScale.y;
        sprite.setScale(flip ? -xScale : xScale, yScale);
    }
    void setVisible(bool isVisible) {
        visible = isVisible;
    }

private:
    TransformComponent* transform;
    sf::Texture texture;
    sf::Sprite sprite;
};