#include "Window.h"

Window::Window(const std::string& title, int width, int  height) : window(sf::VideoMode(width, height), title) {
	window.setFramerateLimit(60);
}

void Window::beginDraw() {
	window.clear(sf::Color::Black);

}

void Window::endDraw() {
	window.display();

}

void Window::processEvents() {
	sf::Event event;
	while (window.pollEvent(event)) {
		if (event.type == sf::Event::Closed)
			window.close();

	}
}
bool Window::isOpen() {
	return window.isOpen();

}
sf::RenderWindow& Window::getRenderWindow() {
	return window;

}