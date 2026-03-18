# RLSudoku

RLSudoku is a C++17 desktop Sudoku game built with raylib, with optional online services for leaderboards, activity tracking, and statistics sync.


## Highlights
- Play Sudoku across multiple difficulty levels.
- Candidate notes and assist tools:
	- auto candidates
	- auto check
	- conflict highlighting
	- candidate drag input and candidate-priority behavior
- Replay and leaderboard views.
- Detailed statistics, including daily breakdowns.
- Activity view with a contribution-style heatmap.
- Optional remote sync for leaderboard/stat/activity data.
- Explicit offline mode toggle to disable all server traffic.

## Tech Stack
- Language: C++17
- Rendering/Input/UI: raylib
- Build system: CMake
- HTTP (client): WinHTTP on Windows, libcurl on non-Windows
- JSON: nlohmann/json
- Puzzle/solver dependency: jczsolve

## Repository Structure
- `src/`: game logic, scenes, UI drawables, storage, sync manager
- `assets/`: fonts and puzzle resources
- `vendor/`: third-party dependencies
- `CMakeLists.txt`: root build configuration

## Requirements
- CMake `>= 3.22`
- A C++17-capable compiler/toolchain
- Git (for submodules)

## Quick Start (Windows PowerShell)

### 1. Clone
```powershell
git clone --recurse-submodules https://github.com/VrikkaDev/RLSudoku.git
cd RLSudoku
```

If already cloned without submodules:
```powershell
git submodule update --init --recursive
```

### 2. Configure + build
```powershell
cmake -S . -B build
cmake --build build
```

### 3. Run
```powershell
.\build\rlsudoku.exe
```


## Runtime Configuration and Data Files
The client creates and uses several files in the working directory:

- `game_data.json`
	- UI options and persisted saveable settings
	- includes toggles like dark mode and offline mode
- `stats.json`
	- local statistics snapshot (including daily stats)
- `remote_sync.json`
	- server endpoint + sync username configuration
- `remote_failed_submissions.jsonl`
	- failed remote submission payload log (if any)
- `leaderboards.bin`
	- local leaderboard storage/cache (legacy/local)

## Gameplay and UX Notes
- The app starts in `MainMenuScene` after loading option saveables.
- UI scales with resolution through `UIHelper` scaling helpers.
- Main menu includes quick status lines for:
	- username
	- server endpoint and client version
	- offline mode
	- leaderboard recording state
	- sync status and update policy state

## Offline Mode
- Offline mode can be toggled in:
	- `Options` scene
	- main menu quick toggle
- When enabled, `RemoteSyncManager`:
	- stops initiating network requests
	- avoids reconnect attempts and keepalive traffic
	- reports disabled sync status
	- keeps local gameplay fully responsive

## License
This project is licensed under the terms in `LICENSE`.
