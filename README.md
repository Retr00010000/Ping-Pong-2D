# Ping Pong 2D (Classic Atari Style in OpenGL)

A classic 2-player 2D Pong game built with C++20, OpenGL 4.6, GLFW, GLAD, and custom engine shaders.

## Controls
- **Player 1 (Left Paddle)**:
  - `W`: Move Up
  - `S`: Move Down
- **Player 2 (Right Paddle)**:
  - `Up Arrow`: Move Up
  - `Down Arrow`: Move Down

## Features
- **Authentic Vintage Aesthetics**:
  - Grainy vintage arcade CRT background texture
  - Vertical dashed center dividing net and crisp boundaries
  - Moving solid white rectangular paddles
  - Tiny classic white square ball with pixel-perfect bounce mechanics
  - Dynamic deflection angles based on paddle impact position
  - Progressive rally ball acceleration
- **On-Screen Win Trackers**:
  - Large retro digital scoreboard for round points (P1 left, P2 right)
  - Match win counters (`WINS: X`) and victory tally blocks on each player's side of the screen
  - Match winner celebration banner (first to 7 points)
  - Non-blocking arcade sound effects using Windows audio beeps
  - Dynamic window title status updates
- **Controls & Hotkeys**:
  - `P`: Freeze / unfreeze the screen (Pause / Resume)
  - `SPACE` / `ENTER`: Play next match after round end
  - `R`: Reset all match wins & scores to 0

## How to Build and Run
### In VS Code
Press `F5` to build and launch immediately.

### In Terminal (PowerShell)
```powershell
.\build.ps1
```
*(Note: If Windows 11 Smart App Control is enabled on your PC, set it to Off under Windows Security -> App & browser control, or enable Developer Mode in Windows Settings).*


