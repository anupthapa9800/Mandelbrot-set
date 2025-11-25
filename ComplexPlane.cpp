#include "ComplexPlane.h"
#include <sstream>
#include <iostream>
#include <cmath> // For std::pow

using namespace sf; // Use the SFML namespace globally

// ----------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------
ComplexPlane::ComplexPlane(int pixelWidth, int pixelHeight)
{
    // Assign m_pixel_size
    m_pixel_size = Vector2i(pixelWidth, pixelHeight);

    // Calculate and assign the aspect ratio (height / width), careful of integer divide
    m_aspectRatio = (float)pixelHeight / (float)pixelWidth;

    // Assign m_plane_center with {0,0}
    m_plane_center = Vector2f(0.0f, 0.0f);

    // Assign m_plane_size with {BASE_WIDTH, BASE_HEIGHT * m_aspectRatio}
    m_plane_size = Vector2f(BASE_WIDTH, BASE_HEIGHT * m_aspectRatio);

    // Assign m_zoomCount with 0
    m_zoomCount = 0;

    // Assign m_State with State::CALCULATING to be ready for the initial screen
    m_state = State::CALCULATING;

    // Initialize VertexArray
    // Set its primitive type to Points and resize it
    m_vArray.setPrimitiveType(Points);
    m_vArray.resize(pixelWidth * pixelHeight);
}

// ----------------------------------------------------------------------
// draw
// ----------------------------------------------------------------------
void ComplexPlane::draw(RenderTarget& target, RenderStates states) const
{
    // Draw the VertexArray
    target.draw(m_vArray);
}

// ----------------------------------------------------------------------
// mapPixelToCoords
// ----------------------------------------------------------------------
Vector2f ComplexPlane::mapPixelToCoords(Vector2i mousePixel)
{
    // Determine the complex plane boundaries based on center and size
    float x_min = m_plane_center.x - m_plane_size.x / 2.0f;
    float x_max = m_plane_center.x + m_plane_size.x / 2.0f;
    float y_min = m_plane_center.y - m_plane_size.y / 2.0f;
    float y_max = m_plane_center.y + m_plane_size.y / 2.0f;

    // Map X (Real component)
    // Pixel range [0, m_pixel_size.x] to Complex range [x_min, x_max]
    float real = ((float)mousePixel.x - 0.0f) / ((float)m_pixel_size.x - 0.0f) * (x_max - x_min) + x_min;

    // Map Y (Imaginary component)
    // Pixel range [m_pixel_size.y, 0] to Complex range [y_min, y_max] (inverted Y axis)
    float imag = ((float)mousePixel.y - (float)m_pixel_size.y) / (0.0f - (float)m_pixel_size.y) * (y_max - y_min) + y_min;

    return Vector2f(real, imag);
}

// ----------------------------------------------------------------------
// countIterations
// ----------------------------------------------------------------------
int ComplexPlane::countIterations(Vector2f coord)
{
    // The complex number c
    std::complex<double> c(coord.x, coord.y);
    // Initial value z0 = 0+c
    std::complex<double> z(0.0, 0.0);
    int i = 0;

    // Iterate the formula z(i+1) = z(i)^2 + c
    // Use std::abs(z * z) < 4.0 for faster execution
    while (std::abs(z * z) < 4.0 && i < MAX_ITER)
    {
        z = z * z + c;
        i++;
    }
    return i;
}

// ----------------------------------------------------------------------
// iterationsToRGB
// ----------------------------------------------------------------------
void ComplexPlane::iterationsToRGB(size_t count, Uint8& r, Uint8& g, Uint8& b)
{
    if (count == MAX_ITER) // Converges (inside the set)
    {
        r = 0; g = 0; b = 0; // Black
    }
    else // Escapes (outside the set, color based on speed of escape)
    {
        // Simple Grayscale Example
        // Map [0, MAX_ITER-1] to [0, 255]
        Uint8 colorValue = (Uint8)(255 * (float)count / MAX_ITER);
        r = colorValue;
        g = colorValue;
        b = colorValue;

        // **Optional: Implement a richer color scheme here based on the instructions**
    }
}

// ----------------------------------------------------------------------
// updateRender
// ----------------------------------------------------------------------
void ComplexPlane::updateRender()
{
    if (m_state == State::CALCULATING) // Only calculate if needed
    {
        int pixelWidth = m_pixel_size.x;
        int pixelHeight = m_pixel_size.y;

        // Loop through all pixels
        for (int i = 0; i < pixelHeight; ++i) // i for y
        {
            for (int j = 0; j < pixelWidth; ++j) // j for x
            {
                // Map the 2D position to its 1D array index
                size_t index = j + i * pixelWidth;
                m_vArray[index].position = { (float)j, (float)i };

                // Find the complex coordinate c
                Vector2f complexCoord = mapPixelToCoords(Vector2i(j, i));

                // Count iterations
                int count = countIterations(complexCoord);

                // Declare three local Uint8 variables r,g,b
                Uint8 r, g, b;

                // Assign the RGB values by reference
                iterationsToRGB(count, r, g, b);

                // Set the color
                m_vArray[index].color = Color(r, g, b);
            }
        }
        // Set the state to DISPLAYING
        m_state = State::DISPLAYING;
    }
}

// ----------------------------------------------------------------------
// zoomIn
// ----------------------------------------------------------------------
void ComplexPlane::zoomIn()
{
    m_zoomCount++; // Increment m_zoomCount

    // Set local variables for the new size using BASE_ZOOM to the m_ZoomCount power
    float zoomFactor = std::pow(BASE_ZOOM, m_zoomCount);
    float newWidth = BASE_WIDTH * zoomFactor;
    float newHeight = BASE_HEIGHT * m_aspectRatio * zoomFactor;

    // Assign m_plane_size with this new size
    m_plane_size = Vector2f(newWidth, newHeight);

    m_state = State::CALCULATING; // Set m_State to CALCULATING
}

// ----------------------------------------------------------------------
// zoomOut
// ----------------------------------------------------------------------
void ComplexPlane::zoomOut()
{
    m_zoomCount--; // Decrement m_zoomCount

    // Same size calculation as zoomIn
    float zoomFactor = std::pow(BASE_ZOOM, m_zoomCount);
    float newWidth = BASE_WIDTH * zoomFactor;
    float newHeight = BASE_HEIGHT * m_aspectRatio * zoomFactor;

    m_plane_size = Vector2f(newWidth, newHeight);

    m_state = State::CALCULATING; // Set m_State to CALCULATING
}

// ----------------------------------------------------------------------
// setCenter
// ----------------------------------------------------------------------
void ComplexPlane::setCenter(Vector2i mousePixel)
{
    // Map the pixel to complex coordinates
    Vector2f newCenter = mapPixelToCoords(mousePixel);

    // Assign m_plane_center with this coordinate
    m_plane_center = newCenter;

    m_state = State::CALCULATING; // Set m_State to CALCULATING
}

// ----------------------------------------------------------------------
// setMouseLocation
// ----------------------------------------------------------------------
void ComplexPlane::setMouseLocation(Vector2i mousePixel)
{
    // Map the pixel to complex coordinates
    Vector2f newLocation = mapPixelToCoords(mousePixel);

    // Assign m_mouseLocation with this coordinate
    m_mouseLocation = newLocation;
}

// ----------------------------------------------------------------------
// loadText
// ----------------------------------------------------------------------
void ComplexPlane::loadText(Text& text)
{
    // Use a stringstream and the corresponding member variables
    std::stringstream ss;
    ss.precision(7); // Use a decent precision for coordinates

    // Create the required output
    ss << "Mandelbrot Set" << std::endl;
    // Center: (m_plane_center.x, m_plane_center.y)
    ss << "Center: (" << m_plane_center.x << "," << m_plane_center.y << ")" << std::endl;
    // Cursor: (m_mouseLocation.x, m_mouseLocation.y)
    ss << "Cursor: (" << m_mouseLocation.x << "," << m_mouseLocation.y << ")" << std::endl;
    ss << "Left-click to Zoom in" << std::endl;
    ss << "Right-click to Zoom out";

    text.setString(ss.str());
}