#pragma once
#include <SFML/Graphics.hpp>

class Window {
public: 
	Window(const std::string& title, int width, int height);
	void beginDraw();
	void endDraw();
	void processEvents();
	bool pollEvent(sf::Event& event);
	void close();
	bool isOpen();
	sf::RenderWindow& getRenderWindow();

private:
	sf::RenderWindow window;


};
