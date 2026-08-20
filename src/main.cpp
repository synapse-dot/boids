/*
 * Copyright (C) 2026 [Your Full Name or Username]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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

namespace boidConfig {
std::vector<Colour> palette = {
    Colour(0.10f, 0.20f, 0.80f), // Deep Blue
    Colour(0.20f, 0.40f, 0.95f), // Royal Blue
    Colour(0.30f, 0.70f, 1.00f), // Sky Blue
    Colour(0.00f, 0.90f, 1.00f), // Cyan
    Colour(0.00f, 0.90f, 0.80f), // Aqua
    Colour(0.20f, 0.90f, 0.60f), // Seafoam
    Colour(0.20f, 0.80f, 0.30f), // Emerald
    Colour(0.50f, 0.95f, 0.40f)  // Mint
};

int numColours = palette.size();
const int defaultColourIdx = 5;
int currentColourIdx = defaultColourIdx;
const int numBoids = 120;

constexpr float maxSpeed = 120.0f;
constexpr float maxAccelerationMag = 100.0f;

constexpr float rPerception = 200.0f;
constexpr float rangeAngle = 120.0f;
const float cosHalfRange = std::cos(rangeAngle / 2);

constexpr float cohesionStrength = 1.0f;
constexpr float weightA = 1.0f;
constexpr float weightC = 1.0f;
constexpr float weightS = 1.0f;
constexpr float boidMass = 1.0f;
constexpr float rSeparation = 30.0f;

constexpr float tipLength = 12.0f;
constexpr float halfWidth = 6.0f;
}; // namespace boidConfig

namespace worldConfig {
float width = 1280.0f;
float height = 720.0f;

Colour bgColour{15, 23, 42};
}; // namespace worldConfig

struct Boid {
  Vec2 position;
  Vec2 velocity;
  Vec2 acceleration;

  Boid(Vec2 position, Vec2 velocity = Vec2(0, 0),
       Vec2 acceleration = Vec2(0, 0), int colorIndex = 0)
      : position(position), velocity(velocity), acceleration(acceleration) {};
};

bool DestroyLastBoid(std::vector<Boid> &boids) {
  if (!boids.empty()) {
    boids.pop_back();
    return true;
  };
  return false;
};

void SpawnBoid(std::vector<Boid> &boids) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> distPosx(0, worldConfig::width);
  std::uniform_real_distribution<float> distPosy(0, worldConfig::height);
  std::uniform_real_distribution<float> distVel(
      -boidConfig::maxSpeed, boidConfig::maxSpeed); // slower start
  std::uniform_real_distribution<float> distAcc(
      -boidConfig::maxAccelerationMag,
      boidConfig::maxAccelerationMag); // slower start
  boids.emplace_back(Vec2(distPosx(gen), distPosy(gen)),
                     Vec2(distVel(gen), distVel(gen)),
                     Vec2(distAcc(gen), distAcc(gen)));
};

std::vector<Boid> CreateBoids() {
  std::vector<Boid> boids;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> distPosx(0, worldConfig::width);
  std::uniform_real_distribution<float> distPosy(0, worldConfig::height);
  std::uniform_real_distribution<float> distVel(-boidConfig::maxSpeed,
                                                boidConfig::maxSpeed);
  std::uniform_real_distribution<float> distAcc(-boidConfig::maxAccelerationMag,
                                                boidConfig::maxAccelerationMag);

  for (int i = 0; i < boidConfig::numBoids; ++i) {
    boids.emplace_back(Vec2(distPosx(gen), distPosy(gen)),
                       Vec2(distVel(gen), distVel(gen)),
                       Vec2(distAcc(gen), distAcc(gen)));
  };

  return boids;
};

void HandleEvent(SDL_Event &event, SDL_Window *window, bool &isRunning,
                 std::vector<Boid> &boids, bool &isPaused) {
  switch (event.type) {
  case SDL_EVENT_QUIT:
    isRunning = false;
    break;

  case SDL_EVENT_KEY_DOWN:
    if (!event.key.repeat) {
      // Handle discrete presses
      switch (event.key.key) {
      case SDLK_B:
        SpawnBoid(boids);
        break;
      case SDLK_D:
        if (!DestroyLastBoid(boids)) {
          std::cerr << "DestroyBoid() failed: boids vector is empty" << '\n';
        };
        break;

      case SDLK_SPACE:
        isPaused = !isPaused;
        break;
      case SDLK_Q:
        isRunning = false;
        break;
      case SDLK_R:
        boids.clear();
        boids = CreateBoids();
        break;
      case SDLK_V:
        if (boidConfig::currentColourIdx == boidConfig::numColours - 1) {
          boidConfig::currentColourIdx = 0;
        } else {
          boidConfig::currentColourIdx += 1;
        };
        break;
      };
    };
    // Handle continuous pressing
    switch (event.key.key) {
    case SDLK_B:
      SpawnBoid(boids);
      break;
    case SDLK_D:
      if (!DestroyLastBoid(boids)) {
        std::cerr << "DestroyBoid() failed: boids vector is empty" << '\n';
      };
      break;
    }

  case SDL_EVENT_WINDOW_RESIZED:
    int width = worldConfig::width;
    int height = worldConfig::height;
    SDL_GetWindowSize(window, &width, &height);
    worldConfig::width = width;
    worldConfig::height = height;
    break;
  };
};

std::vector<Boid *> GetBoidsInView(Boid &boid, std::vector<Boid> &boids) {
  std::vector<Boid *> boidsPerceived;
  for (Boid &other : boids) {
    Vec2 distVector = other.position - boid.position;
    if (distVector.x > worldConfig::width / 2) {
      distVector.x -= worldConfig::width;
    } else if (distVector.x < -worldConfig::width / 2) {
      distVector.x += worldConfig::width;
    }
    if (distVector.y > worldConfig::height / 2) {
      distVector.y -= worldConfig::height;
    } else if (distVector.y < -worldConfig::height / 2) {
      distVector.y += worldConfig::height;
    }
    if ((&boid != &other) &&
        (distVector.MagnitudeSquared() <=
         boidConfig::rPerception * boidConfig::rPerception)) {
      float velMag = boid.velocity.Magnitude();
      float distMag = distVector.Magnitude();
      if (velMag > 0.001f && distMag > 0.001f) {
        if ((boid.velocity.Dot(distVector) / (velMag * distMag)) >=
            boidConfig::cosHalfRange) {
          Boid *ptrOther = &other;
          boidsPerceived.push_back(ptrOther);
        };
      };
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

void UpdateBoidAcceleration(Boid &boid, std::vector<Boid *> &boidsPerceived) {
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

    if (distCentre.x > worldConfig::width / 2) {
      distCentre.x -= worldConfig::width;
    } else if (distCentre.x < -worldConfig::width / 2) {
      distCentre.x += worldConfig::width;
    }
    if (distCentre.y > worldConfig::height / 2) {
      distCentre.y -= worldConfig::height;
    } else if (distCentre.y < -worldConfig::height / 2) {
      distCentre.y += worldConfig::height;
    }

    Vec2 cohesionSteeringForce =
        distCentre.Normalized() * boidConfig::cohesionStrength;

    // 03. Separation
    Vec2 separationSteeringForce(0, 0);
    for (Boid *b : boidsPerceived) {
      Vec2 distVector = boid.position - b->position;
      if (distVector.x > worldConfig::width / 2) {
        distVector.x -= worldConfig::width;
      } else if (distVector.x < -worldConfig::width / 2) {
        distVector.x += worldConfig::width;
      }
      if (distVector.y > worldConfig::height / 2) {
        distVector.y -= worldConfig::height;
      } else if (distVector.y < -worldConfig::height / 2) {
        distVector.y += worldConfig::height;
      }
      float distanceSquared = distVector.MagnitudeSquared();
      if (distanceSquared <=
          boidConfig::rSeparation * boidConfig::rSeparation) {
        separationSteeringForce += distVector / distanceSquared;
      };
    };

    Vec2 steeringForce = alignmentSteeringForce * boidConfig::weightA +
                         cohesionSteeringForce * boidConfig::weightC +
                         separationSteeringForce * boidConfig::weightS;

    boid.acceleration = steeringForce / boidConfig::boidMass;
  }
  LimitMagnitude(boid.acceleration, boidConfig::maxAccelerationMag);
}

void UpdateBoid(Boid &boid, const float dt) { // increased speed

  boid.velocity += boid.acceleration * dt;
  LimitMagnitude(boid.velocity, boidConfig::maxSpeed);
  boid.position += boid.velocity * dt;

  // Wrap Around
  if (boid.position.x > worldConfig::width) {
    boid.position.x -= worldConfig::width;
  } else if (boid.position.x < 0) {
    boid.position.x = boid.position.x + worldConfig::width;
  };
  if (boid.position.y > worldConfig::height) {
    boid.position.y -= worldConfig::height;
  } else if (boid.position.y < 0) {
    boid.position.y = worldConfig::height + boid.position.y;
  };
};

void RenderBackground(SDL_Renderer *renderer) {
  SDL_SetRenderDrawColor(renderer, worldConfig::bgColour.r,
                         worldConfig::bgColour.g, worldConfig::bgColour.b, 255);
  SDL_RenderClear(renderer);
};

void RenderBoid(SDL_Renderer *renderer, const Boid &boid) {
  Vec2 Dir = boid.velocity.Normalized();
  Vec2 Normal = Dir.Perpendicular();
  Vec2 Tip = boid.position - (Dir * boidConfig::tipLength);
  Vec2 A = boid.position + ((Normal * boidConfig::halfWidth) +
                            (Dir * (boidConfig::tipLength * 0.5f)));
  Vec2 B = boid.position - ((Normal * boidConfig::halfWidth) +
                            (Dir * (boidConfig::tipLength * 0.5f)));
  float r = boidConfig::palette[boidConfig::currentColourIdx].r;
  float g = boidConfig::palette[boidConfig::currentColourIdx].g;
  float b = boidConfig::palette[boidConfig::currentColourIdx].b;
  SDL_Vertex vertices[3] = {{{Tip.x, Tip.y}, {r, g, b, 1.0f}},
                            {{A.x, A.y}, {r, g, b, 1.0f}},
                            {{B.x, B.y}, {r, g, b, 1.0f}}};

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

  SDL_Window *window = SDL_CreateWindow(
      "Boids", worldConfig::width, worldConfig::height, SDL_WINDOW_RESIZABLE);
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
  bool isPaused = false;
  SDL_Event event;
  int numBoids = 120;
  std::vector<Boid> boids = CreateBoids();

  float dt;
  Uint64 currentTime;
  Uint64 previousTime = SDL_GetTicks();

  while (isRunning) {
    while (SDL_PollEvent(&event)) {
      HandleEvent(event, window, isRunning, boids, isPaused);
    };

    currentTime = SDL_GetTicks();
    dt = (currentTime - previousTime) / 1000.0f;
    previousTime = currentTime;
    if (!isPaused) {
      for (Boid &boid : boids) {
        std::vector<Boid *> boidsPerceived = GetBoidsInView(boid, boids);
        UpdateBoidAcceleration(boid, boidsPerceived);
      };

      for (Boid &boid : boids) {
        UpdateBoid(boid, dt);
      };
    };
    RenderBackground(renderer);
    for (const Boid &boid : boids) {
      RenderBoid(renderer, boid); // original size
    };

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  };

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
