#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Entity.h"

class Scene {


public:
	Entity* createEntity() {
		auto entity = std::make_unique<Entity>();
		Entity* ptr = entity.get();
		entities.push_back(std::move(entity));
		return ptr;

	}
	std::vector<std::unique_ptr<Entity>>& getEntities() {
		return entities;
	}


	void update(float dt) {
		for (auto& e : entities)
			e->update(dt);

	}
	void render(sf::RenderWindow& window) {
		for (auto& e : entities)
			e->render(window);

	}
	
	
private: 
	std::vector<std::unique_ptr<Entity>> entities;

};