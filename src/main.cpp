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

  Vec2 operator+=(const Vec2 &other) {
    x += other.x;
    y += other.y;
    return *this;
  };

  Vec2 operator-=(const Vec2 &other) {
    x -= other.x;
    y -= other.y;
    return *this;
  };

  Vec2 operator*=(const float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
  };

  Vec2 operator/=(const float scalar) {
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

struct Boid {
  Vec2 position;
  Vec2 velocity;
  Vec2 acceleration;

  Boid(Vec2 position, Vec2 velocity = Vec2(0, 0),
       Vec2 acceleration = Vec2(0, 0))
      : position(position), velocity(velocity), acceleration(acceleration) {};
};

void HandleEvent(SDL_Event &event, bool &isRunning) {
  switch (event.type) {
  case SDL_EVENT_QUIT:
    isRunning = false;
    break;
  };
};

std::vector<Boid> CreateBoids(int numBoids) {
  std::vector<Boid> boids;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> distPos(0, 600);
  std::uniform_real_distribution<float> distVel(-5, 5);

  for (int i = 1; i < (numBoids + 1); ++i) {
    boids.emplace_back(Boid(Vec2(distPos(gen), distPos(gen)),
                            Vec2(distVel(gen), distVel(gen))));
  };

  return boids;
};

std::vector<Boid *> GetBoidsInView(const Boid &boid, std::vector<Boid> &boids,
                                   float perceptionRadius = 180.0f,
                                   float cosHalfRange = 0.65f) {
  std::vector<Boid *> boidsPercieved;
  for (Boid &other : boids) {
    Vec2 distVector = other.position - boid.position;
    if ((&boid != &other) && (distVector.MagnitudeSquared() <=
                              perceptionRadius * perceptionRadius)) {
      if ((boid.velocity.Dot(distVector) /
           (boid.velocity.Magnitude() * distVector.Magnitude())) >=
          cosHalfRange) {
        Boid *ptrOther = &other;
        boidsPercieved.push_back(ptrOther);
      };
    };
  };
  return boidsPercieved;
};

void LimitMagnitude(Vec2 &v, const float maxMagnitude) {
  float magnitudeSquared = v.MagnitudeSquared();
  if (magnitudeSquared > maxMagnitude * maxMagnitude) {
    v = v.Normalized() * maxMagnitude;
  };
};

void UpdateBoid(Boid &boid, const float dt, std::vector<Boid *> &boidsPercieved,
                const float cohesionStrength = 2.5f, const float weightA = 0.9f,
                const float weightC = 0.35f, const float weightS = 2.0f,
                const float boidMass = 1.0f,
                const float separationRadius = 35.0f,
                const float maxSpeed = 140.0f,
                const float maxAcceleration = 250.0f, const int height = 600,
                const int width = 600) {

  // std::cout << "No. of boids percieved by me (" << &boid << " ): " <<
  // boidsPercieved.size() << '\n';

  // 01. Aligment
  Vec2 sumVelocities(0, 0);
  for (Boid *b : boidsPercieved) {
    sumVelocities += b->velocity;
  };

  Vec2 avgFlockVelocity = sumVelocities / boidsPercieved.size();
  Vec2 alignmentSteeringForce = (boid.velocity - avgFlockVelocity);

  // 02. Cohesion
  Vec2 sumPos = 0;
  for (Boid *b : boidsPercieved) {
    sumPos += b->position;
  };
  Vec2 centre = sumPos / boidsPercieved.size();
  Vec2 cohesionSteeringForce =
      (centre - boid.position).Normalized() * cohesionStrength;

  // 03. Separation
  Vec2 separationSteeringForce(0, 0);
  for (Boid *b : boidsPercieved) {
    Vec2 displacement = boid.position - b->position;
    if (displacement.Magnitude() <= separationRadius) {
      separationSteeringForce += displacement / displacement.MagnitudeSquared();
    };
  };

  Vec2 steeringForce = alignmentSteeringForce * weightA +
                       cohesionSteeringForce * weightC +
                       separationSteeringForce * weightS;

  boid.acceleration = steeringForce / boidMass;
  LimitMagnitude(boid.acceleration, maxAcceleration);

  // Update a boid's velocity and position using semi-implicit Euler method.
  boid.velocity += boid.acceleration * dt;
  LimitMagnitude(boid.velocity, maxSpeed);
  boid.position += boid.velocity * dt;

  // Wrap Around
  if (boid.position.x >= width) {
    boid.position.x -= width;
  } else if (boid.position.x <= 0) {
    boid.position.x = width - boid.position.x;
  };
  if (boid.position.y >= height) {
    boid.position.y -= height;
  } else if (boid.position.y <= 0) {
    boid.position.y = height - boid.position.y;
  };
  /*
    std::cout << "I experienced steering force / acceleration <"
              << boid.acceleration.x << ", " << boid.acceleration.y << "> \n
    \n"; std::cout << "My velocity: <" << boid.velocity.x << ", " <<
    boid.velocity.y
              << "> \n \n";
    std::cout << "My position: <" << boid.position.x << ", " << boid.position.y
              << "> \n \n"; */
};

void RenderBackground(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, 15, 23, 42, 255);
  SDL_RenderClear(renderer);
};

void RenderBoid(SDL_Renderer *renderer, const Boid &boid,
                float tipLength = 18.0f, float halfWidth = 9.0f) {
  // The mathematics behind this logic will be explained in
  // "boids-rendering-maths.tex".
  Vec2 Dir = boid.velocity.Normalized();
  Vec2 Normal = Dir.Perpendicular();
  Vec2 Tip = boid.position - (Dir * tipLength);
  Vec2 A = boid.position + ((Normal * halfWidth) + (Dir * (tipLength * 0.5f)));
  Vec2 B = boid.position - ((Normal * halfWidth) + (Dir * (tipLength * 0.5f)));

  SDL_SetRenderDrawColor(renderer, 255, 180, 100, 255);
  SDL_Vertex vertices[3] = {{{Tip.x, Tip.y}, {255, 180, 100, 255}},
                            {{A.x, A.y}, {255, 180, 100, 255}},
                            {{B.x, B.y}, {255, 180, 100, 255}}};

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

  SDL_Window *window =
      SDL_CreateWindow("Boids", 600, 600, SDL_WINDOW_RESIZABLE);
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
  std::vector<Boid> boids = CreateBoids(50);
  float dt;
  Uint64 currentTime;
  Uint64 previousTime = SDL_GetTicks();

  while (isRunning) {
    while (SDL_PollEvent(&event)) {
      HandleEvent(event, isRunning);
    };

    currentTime = SDL_GetTicks();
    dt = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;

    for (Boid &boid : boids) {
      std::vector<Boid *> boidsPercieved = GetBoidsInView(boid, boids);
      UpdateBoid(boid, dt, boidsPercieved);
    };

    RenderBackground(renderer);
    for (const Boid &boid : boids) {
      RenderBoid(renderer, boid);
    };

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  };

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
};
