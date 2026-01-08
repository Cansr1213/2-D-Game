#pragma once
#include <vector>
#include <memory>
#include <algorithm>
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

		entities.erase(
			std::remove_if(entities.begin(), entities.end(),
				[](const std::unique_ptr<Entity>& e) { return !e->isActive(); }),
			entities.end());


	}
	void render(sf::RenderTarget& target) {
		for (auto& e : entities)
			e->render(target);

	}
	
	
private: 
	std::vector<std::unique_ptr<Entity>> entities;

};