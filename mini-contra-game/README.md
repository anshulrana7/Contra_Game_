## Mini Contra-Style 2D Game (C++ / SFML)

This is a small, single-player, 2D side‑scrolling Contra-style demo built with **C++** and **SFML**.

- **Player**: move left/right (`A`/`D` or arrows), jump (`W`/`Up`/`Space`), shoot horizontally (`J`)
- **Lives**: 3 lives; touching an enemy costs one life and respawns you at the start of the level
- **Enemies**: basic AI moving horizontally toward the player; level 2 has faster enemies and a boss
- **Levels**: exactly 2 levels, with simple platform layouts
- **UI**: shows lives, score, and current level at the top-left of the window

### File structure

- `main.cpp` – game window, loop, state management, basic UI
- `Player.h` / `Player.cpp` – player movement, jumping, shooting, bullets, lives & score
- `Enemy.h` / `Enemy.cpp` – enemy types (normal, fast, boss) and simple AI
- `Level.h` / `Level.cpp` – platform layout, enemies, collisions, level completion

### Build instructions (g++)

This project vendors a prebuilt SFML 2.6.1 MinGW package under `external/SFML-2.6.1`, which I have downloaded for you.

- **PowerShell**:

```powershell
.\build.ps1
```

- **cmd.exe**:

```bat
build.bat
```

Both scripts call `g++` with the correct include/lib paths pointing at `external/SFML-2.6.1`.

The code assumes a common Windows font is available at:

```text
C:\Windows\Fonts\arial.ttf
```

If needed, adjust the path in `main.cpp` or load a different font available on your system.

