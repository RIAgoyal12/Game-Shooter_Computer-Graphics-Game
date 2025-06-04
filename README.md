# Space Shooter Game

A classic 2D space shooter game built with OpenGL and GLUT, featuring dynamic enemy types, power-ups, particle effects, and progressive difficulty levels.

## Features

### Core Gameplay
- **Smooth player movement** with WASD keys and arrow keys
- **Multiple weapon types**: Regular bullets and powerful rockets
- **Progressive difficulty**: 10 levels with increasing enemy spawn rates and speed
- **Lives system**: Start with 3 lives, earn bonus lives every 2 levels
- **Dynamic scoring**: Points based on enemy type and weapon used

### Enemy Types
- **Basic Enemy** (Red): 1 health, standard movement
- **Advanced Enemy** (Green): 2 health, sinusoidal movement pattern
- **Elite Enemy** (Blue): 3 health, complex movement patterns

### Power-up System
- **Multi-Shot**: Triple bullet spread for 10 seconds
- **Shield**: Absorbs one collision for 15 seconds
- **Speed Boost**: Double movement speed for 8 seconds
- Power-ups have a 1 in 15 chance to drop from defeated enemies

### Visual Effects
- **Particle systems** for explosions and effects
- **Animated starfield** background with twinkling stars
- **Dynamic explosions** with multi-layered visual effects
- **Animated thruster flames** on player ship
- **Power-up floating animations** with rotation
- **Player invulnerability blinking** after taking damage

## Controls

### Movement
- **WASD** or **Arrow Keys**: Move player ship
- **W/Up**: Move up
- **S/Down**: Move down  
- **A/Left**: Move left
- **D/Right**: Move right

### Combat
- **Spacebar**: Fire bullets
- **R**: Fire rockets (more powerful, larger blast radius)

### Game Management
- **P**: Start new game (when game over)
- **ESC**: Quit game

## Installation & Setup

### Prerequisites
You'll need OpenGL and GLUT libraries installed on your system.

#### Windows
- Install Visual Studio with C++ support
- Install GLUT libraries (freeglut recommended)

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install freeglut3-dev libglu1-mesa-dev
```

#### macOS
```bash
# Using Homebrew
brew install freeglut
```

### Compilation

#### Linux/macOS
```bash
g++ -o space_shooter main.cpp -lGL -lGLU -lglut -lm
```

#### Windows (Visual Studio)
1. Create a new C++ project
2. Add the main.cpp file
3. Link against: `opengl32.lib`, `glu32.lib`, and `freeglut.lib`
4. Build and run

### Running the Game
```bash
./space_shooter
```

## Game Mechanics

### Scoring System
- **Basic Enemy**: 10 points (bullet), 30 points (rocket)
- **Advanced Enemy**: 20 points (bullet), 60 points (rocket)  
- **Elite Enemy**: 30 points (bullet), 90 points (rocket)

### Level Progression
- **Level 1**: 10 enemies to defeat
- **Level 2+**: 10 + (5 × level) enemies to defeat
- **Maximum Level**: 10
- Enemy spawn rate increases with each level
- Enemy movement speed increases by 10% per level

### Power-up Details
- **Drop Rate**: 1 in 15 chance from regular enemies
- **Rocket Bonus**: 2x drop rate when enemies are destroyed by rockets
- **Duration**: Varies by power-up type (8-15 seconds)
- **Visual Indicators**: Active power-ups shown in top-left corner

### Lives & Health
- Start with 3 lives
- Lose 1 life when:
  - Enemy reaches the bottom of the screen
  - Player collides with enemy (unless shielded)
- Gain 1 life every 2 levels
- **Invulnerability Period**: 3 seconds after taking damage
- **Shield**: Absorbs one collision, then deactivates

## Technical Details

### Performance
- **Frame Rate**: 60 FPS (16ms update cycle)
- **Resolution**: 800x600 pixels
- **Particle System**: Dynamic particle generation for effects
- **Collision Detection**: AABB (Axis-Aligned Bounding Box)

### Architecture
- **Object-Oriented Design**: Separate structs for different game objects
- **Component System**: GameObject base class with specialized derivatives
- **State Management**: Global game state with proper cleanup
- **Memory Management**: STL containers with automatic cleanup

## Customization

### Game Constants (located at top of main.cpp)
```cpp
const int POWERUP_CHANCE = 15;      // 1 in X chance for powerup drop
const int MAX_LEVEL = 10;           // Maximum level
const float BULLET_SPEED = 12.0f;   // Bullet movement speed
const float ROCKET_SPEED = 7.0f;    // Rocket movement speed
const float ENEMY_BASE_SPEED = 2.0f; // Base enemy speed
```

### Window Settings
```cpp
const int windowWidth = 800;
const int windowHeight = 600;
```

## Troubleshooting

### Common Issues

**Game won't compile:**
- Ensure GLUT libraries are properly installed
- Check that all header files are accessible
- Verify compiler supports C++11 or later

**Game runs slowly:**
- Check system specifications
- Reduce particle count in `createParticles()` function
- Lower update frequency if needed

**Controls not responsive:**
- Ensure game window has focus
- Check keyboard callback registration
- Verify GLUT event loop is running

### Performance Tips
- The game is optimized for 60 FPS
- Particle effects may impact performance on older hardware
- Consider reducing visual effects if experiencing lag

## Development Team

**Made by: Ria, Shaurya **

## Future Enhancements

Potential improvements for future versions:
- Sound effects and background music
- Additional enemy types and boss battles
- More power-up varieties
- Save/load high scores
- Multiplayer support
- Enhanced graphics and animations
- Mobile platform support


## Contributing

If you'd like to contribute to this project:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

---

Enjoy playing Space Shooter! 🚀✨
