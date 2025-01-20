#include <iostream>
#include <SDL.h>
#include <vector>
#include <cmath>
#include <limits>

// Screen dimensions
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

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
        if (t > 0 && t < 1 && u > 0)
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

// Function to trace rays in all directions
void traceRays(SDL_Renderer *renderer, float originX, float originY, const std::vector<Segment> &scene)
{
    const int NUM_RAYS = 360; // One ray per degree
    const float PI = 3.14159265358979;
    const float MAX_RAY_LENGTH = 1000.0f; // Maximum ray length

    for (int i = 0; i < NUM_RAYS; ++i)
    {
        // Calculate the angle for this ray
        float angle = (i * PI) / 180.0f;

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
            // Draw the ray to the intersection point
            SDL_SetRenderDrawColor(renderer, 255, 255, 102, 255);
            SDL_RenderDrawLine(renderer, originX, originY, closestIntersection.x, closestIntersection.y);
        }
        else
        {
            // No intersection, draw ray to max length
            SDL_SetRenderDrawColor(renderer, 255, 255, 102, 255);
            float endX = originX + std::cos(angle) * MAX_RAY_LENGTH;
            float endY = originY + std::sin(angle) * MAX_RAY_LENGTH;
            SDL_RenderDrawLine(renderer, originX, originY, endX, endY);
        }
    }
}

// Function to render walls in the scene
void renderScene(SDL_Renderer *renderer, const std::vector<Segment> &scene)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Wall color
    for (const Segment &wall : scene)
    {
        SDL_RenderDrawLine(renderer, wall.x1, wall.y1, wall.x2, wall.y2);
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

    // Define the scene with walls
    std::vector<Segment> scene = {
        Segment(400, 400, 500, 500),
        Segment(300, 100, 300, 300),
        Segment(500, 600, 400, 500),
        Segment(300, 300, 100, 300),
        Segment(100, 300, 100, 100)};

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

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render walls and rays
        renderScene(renderer, scene);
        traceRays(renderer, rayOriginX, rayOriginY, scene);

        SDL_RenderPresent(renderer);
    }

    // Cleanup SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}