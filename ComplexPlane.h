#pragma once

#include <SFML/Graphics.hpp>
#include <complex>
#include <thread> // Added for multithreading

using namespace sf;

// Declare the global constants
const unsigned int MAX_ITER = 64;
const float BASE_WIDTH = 4.0;
const float BASE_HEIGHT = 4.0;
const float BASE_ZOOM = 0.5;
// Define the number of threads to use (e.g., matching common core counts)
const unsigned int NUM_THREADS = 8;

// Declare an enum class type named State
enum class State { CALCULATING, DISPLAYING };

class ComplexPlane : public Drawable
{
private:
    VertexArray m_vArray;
    State m_state;
    Vector2f m_mouseLocation; // Complex coordinate of cursor
    Vector2i m_pixel_size; // Pixel width/height
    Vector2f m_plane_center; // Complex center coordinate
    Vector2f m_plane_size; // Complex width/height
    int m_zoomCount;
    float m_aspectRatio;

    // Private helper methods
    int countIterations(Vector2f coord);
    void iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b);
    Vector2f mapPixelToCoords(Vector2i mousePixel);

    // NEW: Function to calculate a specific horizontal strip of the screen
    void calculateStrip(int startRow, int endRow);

    // Override the virtual draw function from sf::Drawable
    virtual void draw(RenderTarget& target, RenderStates states) const;

public:
    // Public methods
    ComplexPlane(int pixelWidth, int pixelHeight);
    void zoomIn();
    void zoomOut();
    void setCenter(Vector2i mousePixel);
    void setMouseLocation(Vector2i mousePixel);
    void loadText(Text& text);
    void updateRender(); // This will be the thread orchestrator
};