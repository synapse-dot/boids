#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <array>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

struct Vec2 {
  float x;
  float y;

  Vec2(float x = 0, float y = 0) : x(x), y(y) {};

  Vec2 operator+(const Vec2 &other) const {
    return Vec2(x + other.x, y + other.y);
  };

  Vec2 operator-(const Vec2 &other) const {
    return Vec2(x - other.x, y - other.y);
  };

  Vec2 operator*(const float scalar) const {
    return Vec2(x * scalar, y * scalar);
  };

  Vec2 operator/(const float scalar) const {
    // When scalar is 0, return <0,0> to avoid division by 0
    if (scalar == 0.0f) {
      return Vec2();
    };

    return Vec2(x / scalar, y / scalar);
  };

  Vec2 &operator+=(const Vec2 &other) {
    x += other.x;
    y += other.y;
    return *this;
  };

  Vec2 &operator-=(const Vec2 &other) {
    x -= other.x;
    y -= other.y;
    return *this;
  };

  Vec2 &operator*=(const float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  };

  Vec2 &operator/=(const float scalar) {
    // Set the vector equal to <0,0> when divided by 0 to avoid errors
    if (scalar == 0) {
      x = 0;
      y = 0;
    } else {
      x /= scalar;
      y /= scalar;
    };
    return *this;
  };

  float Dot(const Vec2 &other) const { return x * other.x + y * other.y; };

  float MagnitudeSquared() const { return (x * x) + (y * y); };

  float Magnitude() const { return std::sqrt((x * x) + (y * y)); };

  Vec2 Normalized() const { return (*this) / Magnitude(); };

  Vec2 Perpendicular() const { return Vec2(-1.0f * y, x); };
};

struct Colour {
    float r;
    float g;
    float b;

    Colour(float r, float g, float b) : r(r), g(g), b(b) {};
};

struct Boid {
  Vec2 position;
  Vec2 velocity;
  Vec2 acceleration;
  int colorIdx;

  Boid(Vec2 position, Vec2 velocity = Vec2(0, 0),
       Vec2 acceleration = Vec2(0, 0), int colorIndex = 0)
      : position(position), velocity(velocity), acceleration(acceleration), colorIdx(colorIndex) {};
};

void CreateBoid(std::vector<Boid>& boids, const int width, const int height) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distPosx(0, width);
    std::uniform_real_distribution<float> distPosy(0, height);
    std::uniform_real_distribution<float> distVel(-100, 100);
    std::uniform_int_distribution<int> distIdx(0,4);
    boids.emplace_back(Vec2(distPosx(gen), distPosy(gen)),
                       Vec2(distVel(gen), distVel(gen)),
                       Vec2(0,0),
                       distIdx(gen));
};

void HandleEvent(SDL_Event &event, SDL_Window* window, bool &isRunning, std::vector<Boid>& boids, int& worldWidth, int& worldHeight) {
  switch (event.type) {
  case SDL_EVENT_QUIT:
    isRunning = false;
    break;

  case SDL_EVENT_KEY_UP:
      CreateBoid(boids, worldWidth, worldHeight);
      break;

  case SDL_EVENT_WINDOW_RESIZED:
      SDL_GetWindowSize(window, &worldWidth, &worldHeight);
      break;
  };
};

std::vector<Boid> CreateBoids(int numBoids, int worldWidth, int worldHeight) {
  std::vector<Boid> boids;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> distPosx(0, worldWidth);
  std::uniform_real_distribution<float> distPosy(0, worldHeight);
  std::uniform_real_distribution<float> distVel(-100, 100);
  std::uniform_int_distribution<int> distIdx(0,4);

  for (int i = 0; i < numBoids; ++i) {
    boids.emplace_back(Vec2(distPosx(gen), distPosy(gen)),
                       Vec2(distVel(gen), distVel(gen)),
                         Vec2(0,0),
                         distIdx(gen));
  };

  return boids;
};

std::vector<Boid *> GetBoidsInView(Boid &boid, std::vector<Boid> &boids,
                                   const float perceptionRadius = 250.0f,
                                   const float cosHalfRange = 0.5f,
                                   const float width = 1000.0f,
                                   const float height = 1000.0f) {
  std::vector<Boid *> boidsPerceived;
  for (Boid &other : boids) {
    Vec2 distVector = other.position - boid.position;
    if (distVector.x > width / 2) {
      distVector.x -= width;
    } else if (distVector.x < -width / 2) {
      distVector.x += width;
    }
    if (distVector.y > height / 2) {
      distVector.y -= height;
    } else if (distVector.y < -height / 2) {
      distVector.y += height;
    }
    if ((&boid != &other) && (distVector.MagnitudeSquared() <=
                              perceptionRadius * perceptionRadius)) {
        float velMag = boid.velocity.Magnitude();
        float distMag = distVector.Magnitude();
        if (velMag > 0.001f && distMag > 0.001f) {
      if ((boid.velocity.Dot(distVector) /
           (velMag * distMag)) >=
          cosHalfRange) {
        Boid *ptrOther = &other;
        boidsPerceived.push_back(ptrOther);
      }; };
    };
  };
  return boidsPerceived;
};

void LimitMagnitude(Vec2 &v, const float maxMagnitude) {
  float magnitudeSquared = v.MagnitudeSquared();
  if (magnitudeSquared > maxMagnitude * maxMagnitude) {
    v = v.Normalized() * maxMagnitude;
  };
};

void UpdateBoidAcceleration(
    Boid &boid, std::vector<Boid *> &boidsPerceived,
    const float cohesionStrength = 5.0f, const float weightA = 0.6f,
    const float weightC = 0.4f, const float weightS = 3.0f,
    const float boidMass = 1.0f, const float separationRadius = 25.0f,
    const float maxAcceleration = 80.0f, const float width = 1000,
    const float height = 1000) {

    // std::cout << "No. of boids percieved by me (" << &boid << " ): " <<
   // boidsPerceived.size() << '\n';
  boid.acceleration = Vec2(0, 0);
  if (!std::empty(boidsPerceived)) {
    // 01. Aligment
    Vec2 sumVelocities(0, 0);
    for (Boid *b : boidsPerceived) {
      sumVelocities += b->velocity;
    };

    Vec2 avgFlockVelocity = sumVelocities / boidsPerceived.size();
    Vec2 alignmentSteeringForce = (avgFlockVelocity - boid.velocity);

    // 02. Cohesion
    Vec2 sumPos(0, 0);
    for (Boid *b : boidsPerceived) {
      sumPos += b->position;
    };
    Vec2 centre = sumPos / boidsPerceived.size();
    Vec2 distCentre = centre - boid.position;

    if (distCentre.x > width / 2) {
      distCentre.x -= width;
    } else if (distCentre.x < -width / 2) {
      distCentre.x += width;
    }
    if (distCentre.y > height / 2) {
      distCentre.y -= height;
    } else if (distCentre.y < -height / 2) {
      distCentre.y += height;
    }

    Vec2 cohesionSteeringForce = distCentre.Normalized() * cohesionStrength;

    // 03. Separation
    Vec2 separationSteeringForce(0, 0);
    for (Boid *b : boidsPerceived) {
      Vec2 distVector = boid.position - b->position;
      if (distVector.x > width / 2) {
        distVector.x -= width;
      } else if (distVector.x < -width / 2) {
        distVector.x += width;
      }
      if (distVector.y > height / 2) {
        distVector.y -= height;
      } else if (distVector.y < -height / 2) {
        distVector.y += height;
      }
      float distanceSquared = distVector.MagnitudeSquared();
      if (distanceSquared <= separationRadius * separationRadius) {
        separationSteeringForce += distVector / distanceSquared;
      };
    };

    Vec2 steeringForce = alignmentSteeringForce * weightA +
                         cohesionSteeringForce * weightC +
                         separationSteeringForce * weightS;

    boid.acceleration = steeringForce / boidMass;
  }
  LimitMagnitude(boid.acceleration, maxAcceleration);
}

void UpdateBoid(Boid &boid, const float dt, const int width = 1000,
                const int height = 1000, const float maxSpeed = 120.0f) {

  // Update a boid's velocity and position using semi-implicit Euler method.
  boid.velocity += boid.acceleration * dt;
  LimitMagnitude(boid.velocity, maxSpeed);
  boid.position += boid.velocity * dt;

  // Wrap Around
  if (boid.position.x >= width) {
    boid.position.x -= width;
  } else if (boid.position.x <= 0) {
    boid.position.x = boid.position.x + width;
  };
  if (boid.position.y >= height) {
    boid.position.y -= height;
  } else if (boid.position.y <= 0) {
    boid.position.y = height + boid.position.y;
  };
/*
    std::cout << "I (" << &boid << " ) experienced steering force / acceleration <"
              << boid.acceleration.x << ", " << boid.acceleration.y << "> \n";
  std::cout << "My velocity: <" << boid.velocity.x << ", " << boid.velocity.y
            << "> \n";
  std::cout << "My position: <" << boid.position.x << ", " << boid.position.y
            << "> \n \n"; */
};

void RenderBackground(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 15, 23, 42, 255);
  SDL_RenderClear(renderer);
};

void RenderBoid(SDL_Renderer *renderer, const Boid &boid, std::array<Colour, 5>& palette,
                float tipLength = 18.0f, float halfWidth = 9.0f){
  // The mathematics behind this logic will be explained in
  // "boids-rendering-maths.tex".
  Vec2 Dir = boid.velocity.Normalized();
  Vec2 Normal = Dir.Perpendicular();
  Vec2 Tip = boid.position - (Dir * tipLength);
  Vec2 A = boid.position + ((Normal * halfWidth) + (Dir * (tipLength * 0.5f)));
  Vec2 B = boid.position - ((Normal * halfWidth) + (Dir * (tipLength * 0.5f)));
  float r = (palette[boid.colorIdx]).r;
  float g = (palette[boid.colorIdx]).g;
  float b =(palette[boid.colorIdx]).b;
  SDL_Vertex vertices[3] = {{{Tip.x, Tip.y}, {r, g, b, 255}},
                            {{A.x, A.y}, {r, g, b, 255}},
                            {{B.x, B.y}, {r, g, b, 255}}};

  std::array<int, 3> indices = {0, 1, 2};
  if (!SDL_RenderGeometry(renderer, nullptr, vertices, 3, &indices[0], 3)) {
    std::cerr << "SDL_RenderGeometry() failed:" << SDL_GetError() << '\n';
  };
};

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "SDL_Init() failed: " << SDL_GetError() << '\n';
    return 1;
  };

  int worldWidth = 800;
  int worldHeight = 600;

  SDL_Window *window =
      SDL_CreateWindow("Boids", worldWidth, worldHeight, SDL_WINDOW_RESIZABLE);
  if (!window) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << '\n';
    SDL_Quit();
    return 1;
  };

  SDL_Renderer *renderer = SDL_CreateRenderer(window, nullptr);
  if (!renderer) {
    std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  };

  bool isRunning = true;
  SDL_Event event;
  std::vector<Boid> boids = CreateBoids(50, worldWidth, worldHeight);
  std::array<Colour, 5> palette = {
      Colour(0, 255, 255),
      Colour(57, 255, 20),
      Colour(255,20,147),
      Colour(255,95,0),
      Colour(255,255,0)
  };
  float dt;
  Uint64 currentTime;
  Uint64 previousTime = SDL_GetTicks();

  while (isRunning) {
    while (SDL_PollEvent(&event)) {
      HandleEvent(event, window, isRunning, boids, worldWidth, worldHeight);
    };

    currentTime = SDL_GetTicks();
    dt = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;

    for (Boid &boid : boids) {
      std::vector<Boid *> boidsPerceived = GetBoidsInView(boid, boids, 250.0f, 0.5f, worldWidth, worldHeight);
      UpdateBoidAcceleration(boid, boidsPerceived, 5.0f, 0.6f, 0.4f, 3.0f, 1.0f, 50.0f, 80.0f, worldWidth, worldHeight);
    };

    for (Boid &boid : boids) {
      UpdateBoid(boid, dt, worldWidth, worldHeight);
    };

    RenderBackground(renderer);
    for (const Boid &boid : boids) {
      RenderBoid(renderer, boid, palette, 9.0f, 4.5f);
    };

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  };

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
};
