# World of Tanks - Single Player Game

## Overview

This project is a single-player game inspired by World of Tanks. Control a tank, destroy enemy tanks, and accumulate points within a time limit.

## Requirements

- **CMake:** Required for building and running the game.

## Gameplay

- **Controls:** W, A, S, D for movement; left-click to fire.
- **Camera:** Positioned behind the tank, following its movements.
- **Enemies:** Move along random paths.
- **Game End:** After at least one minute or when the player's tank is destroyed.

## Implementation

### Visual Elements

- **Tank:** Constructed with complex shapes.
- **Camera Rotation:** Implemented for enhanced gameplay.

### Tank Control

- **Movement:** W, A, S, D keys control tank movement.
- **Turret Rotation:** Allows precise targeting.

### Enemy Behavior

- **Random Movement:** Enemies move unpredictably for a more challenging experience.
- **Enemy Attack:** Enemies target and attack the player when in proximity.

### Collisions

- **Projectile - Tank Collision:** Projectiles disappear on collision, and the tank loses HP.
- **Tank - Tank Collision:** Tanks are moved to avoid collisions.
- **Tank - Building Collision:** Similar approach as tank-tank collisions.

### Visual Effects

- **Enemy Damage:** Deformation and color change based on HP.
