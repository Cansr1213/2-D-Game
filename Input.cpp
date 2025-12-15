#include "Input.h"
#include <SFML/Window/Keyboard.hpp>

bool Input::isKeyPressed(sf::Keyboard::Key key) {
	return sf::Keyboard::isKeyPressed(key);

}