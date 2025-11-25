#include <SFML/Graphics.hpp>
#include "ComplexPlane.h"
#include <iostream>

using namespace sf; // Use the SFML namespace globally
using namespace std;
int main()
{
    // Get the desktop resolution
    VideoMode desktopMode = VideoMode::getDesktopMode();
    unsigned int screenWidth = desktopMode.width;
    unsigned int screenHeight = desktopMode.height;

    // Divide by 2 to start with a smaller, faster screen
    unsigned int pixelWidth = screenWidth / 2;
    unsigned int pixelHeight = screenHeight / 2;

    // Construct the RenderWindow
    RenderWindow window(VideoMode(pixelWidth, pixelHeight), "Mandelbrot Set");
    window.setFramerateLimit(60);

    // Construct the ComplexPlane
    ComplexPlane complexPlane(pixelWidth, pixelHeight);

    // Construct the Font and Text objects
    Font font;
   
    if (!font.loadFromFile("arial.ttf"))
    {
        cerr << "Error loading font file! Make sure 'arial.ttf' is in the executable directory." << std::endl;
        return -1;
    }
    Text text;
    text.setFont(font);
    text.setCharacterSize(18);
    text.setFillColor(Color::White);
    text.setPosition(10.f, 10.f);

    // Begin the main loop
    while (window.isOpen())
    {
        
        // Handle Input segment: Poll Windows queue events
    
        Event event;
        while (window.pollEvent(event))
        {
            // Handle Event::Closed event to close the window
            if (event.type == Event::Closed)
                window.close();

            // Handle Event::MouseButtonPressed
            if (event.type == Event::MouseButtonPressed)
            {
                Vector2i mousePixel = Vector2i(event.mouseButton.x, event.mouseButton.y);

                if (event.mouseButton.button == Mouse::Right)
                {
                    // Right click will zoomOut and call setCenter
                    complexPlane.zoomOut();
                    complexPlane.setCenter(mousePixel);
                }
                else if (event.mouseButton.button == Mouse::Left)
                {
                    // Left click will zoomIn and call setCenter
                    complexPlane.zoomIn();
                    complexPlane.setCenter(mousePixel);
                }
            }

            // Handle Event::MouseMoved
            if (event.type == Event::MouseMoved)
            {
                Vector2i mousePixel = Vector2i(event.mouseMove.x, event.mouseMove.y);
                // Call setMouseLocation to store the (x,y) pixel location
                complexPlane.setMouseLocation(mousePixel);
            }
        }

        // Check if Keyboard::isKeyPressed (Keyboard::Escape) to close the window
        if (Keyboard::isKeyPressed(Keyboard::Escape))
        {
            window.close();
        }

        // ----------------------------------------------------------------------
        // Update Scene segment
        // ----------------------------------------------------------------------
        complexPlane.updateRender(); // Call updateRender
        complexPlane.loadText(text); // Call loadText

        // ----------------------------------------------------------------------
        // Draw Scene segment
        // ----------------------------------------------------------------------
        window.clear(); // Clear the RenderWindow object
        window.draw(complexPlane); // draw the ComplexPlane object
        window.draw(text); // draw the Text object
        window.display(); // Display the RenderWindow object
    }

    return 0;
}