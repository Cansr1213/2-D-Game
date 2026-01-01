#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Component.h"

class Entity {
public:
	void destroy() {
		active = false;

	}
	bool isActive() const {
		return active;

	}
	void update(float dt) {
		if (!active)
			return;

		for (auto& c : components)
			c->update(dt);

	}

	void render(sf::RenderWindow& window) {
		if (!active)
			return;
		for (auto& c : components)
			c->render(window);

	}

	template <typename T, typename... Args>
	T* addComponent(Args&&... args) {
		auto component = std::make_unique<T>(std::forward<Args>(args)...);
		T* ptr = component.get();

		component->entity = this;

		components.push_back(std::move(component));
		return ptr;

	}

	template <typename T> T* getComponent() {
		for (auto& c : components) {
			T* ptr = dynamic_cast<T*>(c.get());
			if (ptr)
				return ptr;

		}
		return nullptr;

	}
private:
	std::vector<std::unique_ptr<Component>> components;
	bool active = true;


};