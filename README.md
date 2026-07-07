# 🎮 Invasion Rush

A 3D rail shooter built with **C++**, **OpenGL**, and **FreeGLUT**, where the player must survive increasingly difficult waves of enemies while aiming with the mouse and managing limited ammunition.

This project was created as part of my OpenGL learning journey to explore real-time graphics, animation, collision detection, game loops, and interactive gameplay mechanics.

---

## Gameplay

The player is positioned in a fixed first-person view while enemies continuously approach from the distance.

There are two enemy types:

* 🤖 **Baymax**
* 🐛 **Caterpillar**

Players must eliminate enemies before they reach the player.

The game becomes progressively harder as levels increase through faster enemies and higher spawn rates.

---

## Features

### 🎯 Combat System

* Mouse aiming with a custom crosshair
* Left-click to shoot
* Limited ammunition
* Right-click to reload
* Headshots deal instant kills
* Body shots require multiple hits

### 👾 Enemy AI

* Multiple enemy types
* Animated movement
* Progressive difficulty
* Random enemy spawning
* Collision detection

### 🎮 Gameplay

* Score system
* Lives system
* Level progression
* Increasing enemy speed
* Increasing enemy spawn rate
* Game Over screen
* Restart functionality

### 🔊 Audio

* Shooting sound effects
* Enemy death sounds
* Special milestone sounds
* Sound overlap prevention

### 🎨 Graphics

* 3D models built entirely with OpenGL primitives
* Animated Baymax character
* Animated Caterpillar character
* Custom HUD
* Custom aiming crosshair

---

## Technologies Used

* C++
* OpenGL
* FreeGLUT
* Windows Multimedia API (`PlaySound`)
* Standard Template Library (STL)

---

## Controls

| Key         | Action                  |
| ----------- | ----------------------- |
| Mouse       | Aim                     |
| Left Click  | Shoot                   |
| Right Click | Reload                  |
| R           | Restart after Game Over |
| Q / ESC     | Quit                    |

---

## Game Mechanics

* Enemies spawn continuously.
* Headshots instantly eliminate enemies.
* Body shots require multiple hits.
* Every enemy that reaches the player removes one life.
* Difficulty increases with each level.
* Survive as long as possible and achieve the highest score.

---

## Concepts Practiced

This project helped me gain experience with:

* OpenGL rendering
* Real-time game loops
* Animation
* Collision detection
* Object management
* Mouse interaction
* Keyboard input
* 3D transformations
* STL vectors
* State management
* Audio programming
* Basic game design

---

## Project Structure

```text
main.cpp
shoot.wav
die.wav
wady.wav
ash.wav
```

---

## Future Improvements

If I revisit this project, I plan to:

* Add textured 3D models
* Implement proper lighting
* Add particle effects
* Add multiple weapons
* Add additional enemy types
* Improve enemy AI
* Introduce power-ups
* Save high scores
* Replace primitive models with imported meshes
* Refactor the project into multiple source files and classes

---


## What I Learned

This project taught me much more than simply drawing objects with OpenGL.

I learned how to combine rendering, animation, collision detection, input handling, audio, and gameplay mechanics into a complete interactive application. It also gave me a deeper appreciation for how many independent systems work together inside even a relatively small game.

Looking back, this project marks an important step in my journey from learning graphics programming to building complete software systems.

---

## License

This project is intended for educational purposes.
