# pubg_rangefinder

`pubg_rangefinder` is a Windows desktop shell for a future PUBG-specific screen rangefinder.

## Stage 1

Current scope:

- `C++ + Qt Widgets`
- Settings window UI scaffold
- System tray scaffold
- Transparent overlay window scaffold
- GitHub beta release workflow

Out of scope for this stage:

- Actual ranging logic
- Global hotkey capture
- Mouse hooks
- Config persistence
- Map scaling algorithms

## Build

Requirements:

- CMake 3.21+
- Python for Windows
- Visual Studio 2022 C++ toolchain
- Git Bash on Windows

The local build script downloads Qt into `build_cache/`, so Qt does not need to be installed system-wide.

Run from Git Bash:

```bash
bash build.sh
```

## Notes

- The local build script reuses cached downloads under `build_cache/`.
- The runnable output is written to `build/package/`.
- Qt runtime files are collected into `build/package/` by `windeployqt`.
- The app is expected to run on Windows only.
