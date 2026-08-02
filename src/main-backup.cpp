#include <SDL3/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>

// 2D Vector
struct Vec2 {
    float x;
    float y;

    Vec2(float x = 0.0f, float y = 0.0f) : x(x), y(y)
    {}

    Vec2 operator* (const float scalar) const {
        return Vec2(x * scalar, y * scalar);
    }

    Vec2 operator/ (const float scalar) const {
        if (scalar==0){
            // TODO: raise an exception
        }
        return Vec2(x / scalar, y / scalar);
    }

    Vec2 operator+ (const Vec2 &other) const {
        return Vec2(x + other.x, y + other.y);
    }

    Vec2 operator- (const Vec2 &other) const {
        return Vec2(x - other.x, y - other.y);
    }

    Vec2& operator+= (const Vec2 &other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    float Length() const {
        return std::sqrt(x * x + y * y);
    }

    float LengthSquared() const {
        return (x * x + y * y);
    }

    Vec2 Normalized() const{
        if ((*this).LengthSquared() == 0) {
            // Raise exception
        }
        return *this / Length();
    }

    float DotProduct(const Vec2& other) const {
        return x * other.x + y * other.y;
    }

    Vec2 operator- () const {
        return Vec2(-x, -y);
    }

    Vec2& operator-=(const Vec2& other){
        x -= other.x;
        y -= other.y;
        return *this;
    }

    Vec2& operator*=(float scalar){
        x *= scalar;
        y *= scalar;
        return *this;
    }

    Vec2& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        return *this;
    }

};

// A boid
struct Boid {
    Vec2 position;
    Vec2 velocity;
    Vec2 acceleration;

    Boid(Vec2 position, Vec2 velocity, Vec2 acceleration = Vec2(0,0)):
        position(position),
        velocity(velocity),
        acceleration(acceleration)
    {}
};

// A function to create a vector containing boids
std::vector<Boid> CreateBoids (int numBoids) {
    std::vector<Boid> boids;
    for (int i = 0; i < numBoids; ++i) {
        boids.emplace_back(
            Vec2(i,i*i),
            Vec2(2.0f*i, 1.5f*i),
            Vec2(i/2.0f, i/1.5f)
            );
    };

    return boids;
}

void HandleEvents(const SDL_Event& event, bool& running) {
    switch(event.type){
    case SDL_EVENT_QUIT :
        running = false;
        break;
    };
}

void UpdateBoid(const float dt, Boid& boid) {
    boid.velocity += boid.acceleration * dt;
    boid.position += boid.velocity * dt;
}

void DrawBg(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    SDL_RenderClear(renderer);
}

void DrawBoid(SDL_Renderer* renderer, const Boid& boid) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_FRect rect(boid.position.x, boid.position.y, 10, 10);
    SDL_RenderFillRect(renderer, &rect);
}

int main () {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    };

    SDL_Window* window = SDL_CreateWindow(
        "Boids Simulation",
        600,
        600,
        SDL_WINDOW_RESIZABLE
        );

    if (!window) {
        std::cerr << "SDL_CreateWindow Failed: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    };

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    };

    bool running = true;
    SDL_Event event;

    std::vector<Boid> boids = CreateBoids(10);

    Uint64 previousTime = SDL_GetTicks();

    // The main loop
    while (running) {
        while (SDL_PollEvent(&event)) {
            HandleEvents(event, running);
        };

        Uint64 currentTime = SDL_GetTicks();
        float dt = (currentTime - previousTime) / 1000.0f;
        previousTime = currentTime;
        for (Boid& boid : boids) {
            UpdateBoid(dt, boid);
        };

        DrawBg(renderer);
        for (Boid& boid : boids) {
            DrawBoid(renderer, boid);
        };

        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    };

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}