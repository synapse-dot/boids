# Boids Version 1.0

A 2D Simulation of flocking behaviour of birds based on 3 rules of alignment, separation and cohesion that responds to the user  through keyboard controls

## Features
- Real-time 2D flocking simulation.
- Interactive keyboard controls (see below) to spawn, delete, reset, pause, and recolor boids.

## How to Build from Source

- Install SDL3. Eg (for Arch users) :
` sudo pacman -S SDL3 `
- Install other prerequisites such as cmake, g++.
- In the terminal:
``` git clone https://github.com/synapse-dot/boids
cd boids
cmake -S . -B build
cmake --build build 
```

- Execute with:
` ./build/boids`

## Keyboard Controls

| Key Pressed | Result |
|-----|---------------------------------------------------------|
|b|Create a new boid with a random velocity at a random position |
|d|Delete the boid created last|
|r|Reset the simulation|
|SPACE | Pause/restart the simulation |
| v| Change the colors of boids|



## Known Limitations

- Deleting boids when none exist (d key) is a no-op (safe, but silent).
- Performance may drop above ~300 boids on older hardware.

## License
This project is released under the **GNU General Public License v3.0**. 
You are free to use, modify, and distribute this software, but any 
distributed copies or modifications must also be open-sourced under 
the same license. See the [LICENSE](LICENSE) file for full details.
