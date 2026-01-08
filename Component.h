#pragma once
#include <SFML/Graphics.hpp>
class Entity;

class Component {
public:
	Entity* entity = nullptr; // REQUIRED
	virtual ~Component() = default;
	virtual void update(float dt) {}
	virtual void render(sf::RenderTarget& target) {}
};
