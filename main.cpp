#include <iostream>
#include <SDL.h>
#include <vector>
#include <cmath>
#include <limits>

// Screen dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PI 3.14159265358979f
#define ANGLE_STEP_DEG 0.05f

SDL_Texture *gTexture = nullptr;
void *gPixels = nullptr;
int gPitch = 0;
int gPixelPerRow = 0;
Uint32 *gPixelBuffer = nullptr;

// Point structure to represent positions
struct Point
{
    float x, y;
};

// Segment structure to represent walls
struct Segment
{
    float x1, y1, x2, y2;

    Segment(float x1, float y1, float x2, float y2)
        : x1(x1), y1(y1), x2(x2), y2(y2) {}
};

// Define the scene with walls
std::vector<Segment> scene = {
    Segment(400, 400, 500, 500),
    Segment(300, 100, 300, 300),
    Segment(500, 600, 400, 500),
    Segment(300, 300, 100, 300),
    Segment(100, 300, 100, 100),
    Segment(600, 150, 600, 450), // mur vertical à droite
    Segment(200, 450, 200, 150)  // mur vertical à gauche
};

// Ray class to handle ray operations
class Ray
{
public:
    Point pos; // Starting point of the ray
    Point dir; // Normalized direction vector

    // Constructor to initialize ray position and angle
    Ray(float x, float y, float angle)
    {
        pos = {x, y};
        dir = {std::cos(angle), std::sin(angle)};
    }

    // Cast ray against a wall and find intersection
    Point cast(const Segment &wall)
    {
        float x1 = wall.x1, y1 = wall.y1;
        float x2 = wall.x2, y2 = wall.y2;
        float x3 = pos.x, y3 = pos.y;
        float x4 = pos.x + dir.x, y4 = pos.y + dir.y;

        // Calculate determinant
        float den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
        if (den == 0)
        {
            // Parallel or collinear, no intersection
            return {std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
        }

        // Solve for t and u
        float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den;
        float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / den;

        // Check if intersection is valid
        if (t >= 0 && t <= 1 && u >= 0)
        {
            // Calculate intersection point
            float px = x1 + t * (x2 - x1);
            float py = y1 + t * (y2 - y1);
            return {px, py};
        }

        // No valid intersection
        return {std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
    }
};

void drawRay(float x1, float y1, float angle, float distance)
{
    float k = 0.005f;

    float stepSize = 1.0f;

    float stepX = std::cos(angle) * stepSize;
    float stepY = std::sin(angle) * stepSize;
    float currentX = x1;
    float currentY = y1;
    for (float d = 0.0f; d <= distance; d += stepSize)
    {
        float attenuation = expf(-k * d); // Où k est une constante positive
        attenuation = std::max(0.0f, std::min(1.0f, attenuation));
        Uint8 alpha = static_cast<Uint8>(attenuation * 255.0f);

        if (alpha == 0)
        {
            break;
        }

        Uint32 pixelColor = (alpha << 24) | (255 << 16) | (255 << 8) | 102;

        int drawX = static_cast<int>(currentX);
        int drawY = static_cast<int>(currentY);

        if (drawX <= 0 || drawX >= SCREEN_WIDTH || drawY <= 0 || drawY >= SCREEN_HEIGHT)
        {
            break;
        }

        gPixelBuffer[drawY * gPixelPerRow + drawX] = pixelColor;

        currentX += stepX;
        currentY += stepY;
    }
}

void clearTexture()
{
    for (int y = 0; y < SCREEN_HEIGHT; ++y)
    {
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            gPixelBuffer[y * gPixelPerRow + x] = 0xFF000000;
        }
    }
}

// Function to trace rays in all directions
void traceRays(float originX, float originY)
{
    const float ANGLE_STEP_RAD = ANGLE_STEP_DEG * PI / 180.0f;

    // We want to cover 360 degrees
    const int NUM_RAYS = static_cast<int>(360.0f / ANGLE_STEP_DEG);

    for (int i = 0; i < NUM_RAYS; ++i)
    {
        // Calculate the angle for this ray
        float angle = i * ANGLE_STEP_RAD;

        // Create a ray at the given angle
        Ray ray(originX, originY, angle);

        Point closestIntersection = {std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
        float closestDistance = std::numeric_limits<float>::infinity();

        // Check the ray against all walls
        for (const Segment &wall : scene)
        {
            Point intersection = ray.cast(wall);
            float distance = hypot(intersection.x - originX, intersection.y - originY);

            if (distance < closestDistance)
            {
                closestIntersection = intersection;
                closestDistance = distance;
            }
        }

        if (closestDistance < std::numeric_limits<float>::infinity())
        {
            drawRay(originX, originY, angle, closestDistance);
        }
        else
        {
            // No intersection, draw ray to max length
            drawRay(originX, originY, angle, closestDistance);
        }
    }
}

// Draw a line using Bresenham’s algorithm
void drawLine(int x1, int y1, int x2, int y2, Uint32 color)
{
    int dx = std::abs(x2 - x1);
    int dy = std::abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (true)
    {
        if (x1 >= 0 && x1 < SCREEN_WIDTH && y1 >= 0 && y1 < SCREEN_HEIGHT)
            gPixelBuffer[y1 * gPixelPerRow + x1] = color;

        if (x1 == x2 && y1 == y2)
            break;

        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dx)
        {
            err += dx;
            y1 += sy;
        }
    }
}

void drawWalls()
{
    // Draw all walls on the same locked texture:
    for (const Segment &wall : scene)
    {
        // Draw this wall line using Bresenham's algorithm
        int x1 = static_cast<int>(wall.x1);
        int y1 = static_cast<int>(wall.y1);
        int x2 = static_cast<int>(wall.x2);
        int y2 = static_cast<int>(wall.y2);
        drawLine(x1, y1, x2, y2, 0xFFFFFFFF);
    }
}

int main()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window *window = SDL_CreateWindow("2D Ray Casting", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
    {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    gTexture = SDL_CreateTexture(renderer,
                                 SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 SCREEN_WIDTH,
                                 SCREEN_HEIGHT);

    SDL_SetTextureBlendMode(gTexture, SDL_BLENDMODE_BLEND);

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
        }

        // Get mouse position for ray origin
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        float rayOriginX = static_cast<float>(mouseX);
        float rayOriginY = static_cast<float>(mouseY);

        if (SDL_LockTexture(gTexture, nullptr, &gPixels, &gPitch) < 0)
        {
            std::cerr << "SDL_LockTexture Error: " << SDL_GetError() << std::endl;
            return 0;
        }

        gPixelPerRow = gPitch / sizeof(Uint32);
        gPixelBuffer = static_cast<Uint32 *>(gPixels);
        clearTexture();

        // Render walls and rays
        traceRays(rayOriginX, rayOriginY);
        drawWalls();

        SDL_UnlockTexture(gTexture);

        SDL_RenderCopy(renderer, gTexture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    // Cleanup SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_DestroyTexture(gTexture);
    SDL_Quit();

    return 0;
}