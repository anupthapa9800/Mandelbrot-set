#include "ComplexPlane.h"
#include <sstream>
#include <iostream>
#include <cmath> 
#include <vector> // Required for storing thread objects
#include <numeric> // Optional: Useful for certain loop calculations

using namespace sf;

// ----------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------
ComplexPlane::ComplexPlane(int pixelWidth, int pixelHeight)
{
    // Initialize member variables
    m_pixel_size = Vector2i(pixelWidth, pixelHeight);
    m_aspectRatio = (float)pixelHeight / (float)pixelWidth;
    m_plane_center = Vector2f(0.0f, 0.0f);
    m_plane_size = Vector2f(BASE_WIDTH, BASE_HEIGHT * m_aspectRatio);
    m_zoomCount = 0;
    m_state = State::CALCULATING;

    // Initialize VertexArray
    m_vArray.setPrimitiveType(Points);
    m_vArray.resize(pixelWidth * pixelHeight);
}

// ----------------------------------------------------------------------
// draw
// ----------------------------------------------------------------------
void ComplexPlane::draw(RenderTarget& target, RenderStates states) const
{
    target.draw(m_vArray);
}


// ----------------------------------------------------------------------
// updateRender (Multithreading Orchestrator)
// ----------------------------------------------------------------------
void ComplexPlane::updateRender()
{
    if (m_state == State::CALCULATING)
    {
        int pixelHeight = m_pixel_size.y;

        // Vector to store the running thread objects
        std::vector<std::thread> threads;

        // Determine the height of each strip
        int stripHeight = pixelHeight / NUM_THREADS;
        int remainingRows = pixelHeight % NUM_THREADS;

        int currentRow = 0;

        // Loop to create and start the threads
        for (unsigned int k = 0; k < NUM_THREADS; ++k)
        {
            int startRow = currentRow;
            int endRow = currentRow + stripHeight;

            // Distribute remaining rows evenly
            if (k < remainingRows)
            {
                endRow++;
            }

            // Launch the thread to calculate its assigned strip
            // The &ComplexPlane::calculateStrip is the member function pointer, 
            // 'this' is the object instance, and startRow/endRow are the arguments.
            threads.emplace_back(&ComplexPlane::calculateStrip, this, startRow, endRow);

            currentRow = endRow;
        }

        // Wait for all threads to finish their work before proceeding
        for (auto& thread : threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }

        m_state = State::DISPLAYING;
    }
}


// ----------------------------------------------------------------------
// zoomIn
// ----------------------------------------------------------------------
void ComplexPlane::zoomIn()
{
    m_zoomCount++;
    float zoomFactor = std::pow(BASE_ZOOM, m_zoomCount);
    float newWidth = BASE_WIDTH * zoomFactor;
    float newHeight = BASE_HEIGHT * m_aspectRatio * zoomFactor;
    m_plane_size = Vector2f(newWidth, newHeight);
    m_state = State::CALCULATING;
}


// ----------------------------------------------------------------------
// zoomOut
// ----------------------------------------------------------------------
void ComplexPlane::zoomOut()
{
    m_zoomCount--;
    float zoomFactor = std::pow(BASE_ZOOM, m_zoomCount);
    float newWidth = BASE_WIDTH * zoomFactor;
    float newHeight = BASE_HEIGHT * m_aspectRatio * zoomFactor;
    m_plane_size = Vector2f(newWidth, newHeight);
    m_state = State::CALCULATING;
}


// ----------------------------------------------------------------------
// setCenter
// ----------------------------------------------------------------------
void ComplexPlane::setCenter(Vector2i mousePixel)
{
    Vector2f newCenter = mapPixelToCoords(mousePixel);
    m_plane_center = newCenter;
    m_state = State::CALCULATING;
}


// ----------------------------------------------------------------------
// setMouseLocation
// ----------------------------------------------------------------------
void ComplexPlane::setMouseLocation(Vector2i mousePixel)
{
    Vector2f newLocation = mapPixelToCoords(mousePixel);
    m_mouseLocation = newLocation;
}


// ----------------------------------------------------------------------
// loadText
// ----------------------------------------------------------------------
void ComplexPlane::loadText(Text& text)
{
    std::stringstream ss;
    ss.precision(7);
    ss << "Mandelbrot Set (Multithreaded)" << std::endl;
    ss << "Center: (" << m_plane_center.x << "," << m_plane_center.y << ")" << std::endl;
    ss << "Cursor: (" << m_mouseLocation.x << "," << m_mouseLocation.y << ")" << std::endl;
    ss << "Left-click to Zoom in" << std::endl;
    ss << "Right-click to Zoom out";
    text.setString(ss.str());
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
    // Stop if |z|^2 > 4.0 or max iterations reached
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
        // Simple Grayscale Color Scheme
        // Map iteration count [0, MAX_ITER-1] to a grayscale value [0, 255]
        Uint8 colorValue = (Uint8)(255 * (float)count / MAX_ITER);
        r = colorValue;
        g = colorValue;
        b = colorValue;
    }
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

    // Map X (Real component): [0, pixelWidth] -> [x_min, x_max]
    float real = ((float)mousePixel.x / (float)m_pixel_size.x) * (x_max - x_min) + x_min;

    // Map Y (Imaginary component): [pixelHeight, 0] -> [y_min, y_max] (inverted Y axis)
    // The formula handles the screen's inverted Y axis (y=0 at the top).
    float imag = (1.0f - (float)mousePixel.y / (float)m_pixel_size.y) * (y_max - y_min) + y_min;

    return Vector2f(real, imag);
}



// ----------------------------------------------------------------------
// calculateStrip (Private Multithreading Helper)
// ----------------------------------------------------------------------
void ComplexPlane::calculateStrip(int startRow, int endRow)
{
    int pixelWidth = m_pixel_size.x;

    // Loop through the assigned vertical strip (rows)
    for (int i = startRow; i < endRow; ++i)
    {
        for (int j = 0; j < pixelWidth; ++j) // j for x (full width)
        {
            size_t index = j + i * pixelWidth;
            m_vArray[index].position = { (float)j, (float)i };

            // Find the complex coordinate c
            Vector2f complexCoord = mapPixelToCoords(Vector2i(j, i));

            // Count iterations
            int count = countIterations(complexCoord);

            Uint8 r, g, b;

            // Assign the RGB values by reference
            iterationsToRGB(count, r, g, b);

            // Set the color
            m_vArray[index].color = Color(r, g, b);
        }
    }
}





