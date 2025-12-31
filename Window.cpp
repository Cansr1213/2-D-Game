#include "Window.h"


Window::Window(const std::string& title, int width, int  height) : window(sf::VideoMode(width, height), title) {
	window.setFramerateLimit(60);
}

void Window::beginDraw() {
	window.clear(sf::Color(120, 180, 225));

}

void Window::endDraw() {
	window.display();

}

void Window::processEvents() {
	sf::Event event;
	while (window.pollEvent(event)) {
		if (event.type == sf::Event::Closed)window.close();
		
		if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)window.close();
	}
}
bool Window::pollEvent(sf::Event& event) {
	return window.pollEvent(event);

}
void Window::close() {
	window.close();


}
bool Window::isOpen() {
	return window.isOpen();

}
sf::RenderWindow& Window::getRenderWindow() {
	return window;

}