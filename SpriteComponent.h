#pragma once
#include "Component.h"
#include "TransformComponent.h"
#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>

class SpriteComponent : public Component {
public:
    int frameWidth = 64;
    int frameHeight = 64;
    bool flipped = false;

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
        sprite.setPosition(transform->position);
    }

    void render(sf::RenderWindow& window) override {
        window.draw(sprite);
    }

    sf::Sprite& getSprite() { return sprite; }

    // ✅ Flip using scale ONLY
    void setFlipped(bool flip) {
        if (flipped == flip) return;
        flipped = flip;
        sprite.setScale(flip ? -1.f : 1.f, 1.f);
    }

private:
    TransformComponent* transform;
    sf::Texture texture;
    sf::Sprite sprite;
};
