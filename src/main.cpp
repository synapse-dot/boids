#include <iostream>
#include <SDL3/SDL.h>
#include <vector>
#include <cmath>
#include <cassert>

struct Vec2 {
    float x;
    float y;

    Vec2(float x = 0, float y = 0) : x(x), y(y) {
    }

    Vec2 operator* (const float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }

    Vec2 operator/ (const float scalar) const {
        assert(scalar != 0);
        return Vec2(x / scalar, y / scalar);
    }

    Vec2 operator+ (const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator- (const Vec2& other) const {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2& operator+= (const Vec2& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    Vec2& operator-= (const Vec2& other){
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2& operator*= (const float scalar) {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vec2& operator/= (const float scalar) {
        assert(scalar != 0);
        x /= scalar;
        y /= scalar;
        return *this;
    }

    float sqrMagnitude () const {
        return (x * x) + (y * y);
    }

    float Magnitude () const {
        return std::sqrt((x * x)+(y * y));
    }

    Vec2 Normalized () const {
        float magnitude = Magnitude();
        float normalizedx = x / magnitude;
        float normalizedy = y / magnitude;
        return Vec2(normalizedx,normalizedy);
    }

    float Dot (const Vec2& other) const {
        return (other.x * x) + (other.y * y);
    }
};

struct Boid {
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;

    Boid (Vec2 position, Vec2 velocity, Vec2 acceleration) :
        position(position), velocity(velocity), acceleration(acceleration)
    {
    }
};

void handleEvent(SDL_Event& event, bool& running) {
    switch (event.type) {
    case SDL_EVENT_QUIT :
        running = false;
    }
}

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    };

    SDL_Window* window = SDL_CreateWindow("Boids",
                                          600,
                                          600,
                                          SDL_WINDOW_RESIZABLE
                                          );

    if (!window) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while(running) {
        while(SDL_PollEvent(&event)) {
            handleEvent(event, running);
        }
    }

    return 0;
}