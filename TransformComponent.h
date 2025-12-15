#pragma once
#include <SFML/Graphics.hpp>
#include "Component.h"


class TransformComponent : public Component {
public: 
	sf::Vector2f position;
	sf::Vector2f scale = { 1.f, 1.f };
	float rotation = 0.f;

	TransformComponent(float x, float y) : 
		position(x, y) {} 

	void update(float dt) override {
		// Movement logic later

	}

};
