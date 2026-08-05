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

    float magnitude () const {
        return std::sqrt((sqrMagnitude()));
    }

    Vec2 normalized () const {
        float length = magnitude();
        assert(length != 0);
        return *this / length;
    }

    float dot (const Vec2& other) const {
        return (other.x * x) + (other.y * y);
    }

    Vec2 operator-() const {
        return Vec2(-(this->x),-(this->y));
    }
};

struct Boid {
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;

    Boid (Vec2 position = Vec2(0,0),
         Vec2 velocity = Vec2(0,0),
         Vec2 acceleration = Vec2(0,0)
         ) :
        position(position),
        velocity(velocity),
        acceleration(acceleration)
    {
    }
};

std::vector<Boid> createBoids(int numBoids) {
    std::vector<Boid> boids;
    for (int i = 0; i < numBoids; ++i) {
        boids.emplace_back(Boid(Vec2(5.0f * i, 25.0f * i), Vec2(10.0f * i, 20.0f * i)));
    }
    return boids;
}

void handleEvent(SDL_Event& event, bool& running) {
    switch (event.type) {
    case SDL_EVENT_QUIT :
        running = false;
    }
}

void drawBackground(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 12, 18, 38, 255);
    SDL_RenderClear(renderer);
}

void updateBoid(Boid& boid, float dt) {
    boid.velocity += boid.acceleration * dt;
    boid.position += boid.velocity * dt;
}

void drawBoid(SDL_Renderer* renderer, Boid& boid) {
    SDL_SetRenderDrawColor(renderer, 240, 240, 255, 255);
    SDL_FRect rect;
    rect.x = boid.position.x;
    rect.y = boid.position.y;
    rect.w = 10;
    rect.h = 10;
    SDL_RenderFillRect(renderer, &rect);
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
    std::vector boids = createBoids(3);
    Uint64 previousTime = SDL_GetTicks();

    while(running) {
        while(SDL_PollEvent(&event)) {
            handleEvent(event, running);
        }

        drawBackground(renderer);

        Uint64 currentTime = SDL_GetTicks();
        float dt = (currentTime - previousTime)/ 1000.0f;
        previousTime = currentTime;

        for (Boid& boid : boids) {
            updateBoid(boid, dt);
        }

        for (Boid& boid : boids) {
            drawBoid(renderer, boid);
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}