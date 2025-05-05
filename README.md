<div align="center">
  <img src="ResoChan.png"  width="200"/>
  <h1 align="center">ResoChan!</h1>
  <h4 align="center">Single-file Resolution Changer for Windows</h4>
</div>

<div align="center">
  <a href="https://github.com/HiSkyZen/ResoChan"><img src="https://img.shields.io/github/stars/hiskyzen/resochan.svg?logo=github&style=for-the-badge" alt="GitHub stars"></a>
  <a href="https://github.com/HiSkyZen/ResoChan/releases/latest"><img src="https://img.shields.io/github/downloads/hiskyzen/resochan/total.svg?style=for-the-badge&logo=github" alt="GitHub Releases"></a>
</div>

## 🖥️ About

**ResoChan** is a lightweight Windows command-line utility for changing and testing display resolutions and refresh rates with ease.  
It supports both intuitive parameter syntax and legacy QRes-compatible arguments, and can automatically detect the display under your mouse cursor.

---

## ✨ Features

- 🔍 List all connected displays (`--list`)
- 📐 Show supported display modes (`--modes`)
- ✅ Test resolution and refresh rate support without applying (`--test`)
- 📺 Change resolution and refresh rate (`-w`, `-h`, `-r`)
- 🖱️ Automatically targets the display under the cursor if none is specified
- 🔁 Fully supports QRes-style arguments like `/x:1920 /y:1080 /r:60`

---

## 🧩 Usage

### Set resolution and refresh rate
```pwsh
.\resochan.exe [<display>] -w <width> -h <height> [-r <refreshrate>]
.\resochan.exe -w 1920 -h 1080
.\resochan.exe -w 1920 -h 1080 -r 60
.\resochan.exe 2 -w 1920 -h 1080
.\resochan.exe 2 -w 1920 -h 1080 -r 60
.\resochan.exe \\.\DISPLAY2 -w 1920 -h 1080
.\resochan.exe \\.\DISPLAY2 -w 1920 -h 1080 -r 60

.\resochan.exe [<display>] --width <width> --height <height> [--refresh <refreshrate>]
.\resochan.exe --width 1920 --height 1080
.\resochan.exe --width 1920 --height 1080 --refresh 60
.\resochan.exe 2 --width 1920 --height 1080
.\resochan.exe 2 --width 1920 --height 1080 --refresh 60
.\resochan.exe \\.\DISPLAY2 --width 1920 --height 1080
.\resochan.exe \\.\DISPLAY2 --width 1920 --height 1080 --refresh 60

.\resochan.exe [<display>] /x:<width> /y:<height> [/r:<refreshrate>]
.\resochan.exe /w:1920 /y:1080
.\resochan.exe /w:1920 /y:1080 /r:60
.\resochan.exe 2 /w:1920 /y:1080
.\resochan.exe 2 /w:1920 /y:1080 /r:60
.\resochan.exe \\.\DISPLAY2 /w:1920 /y:1080
.\resochan.exe \\.\DISPLAY2 /w:1920 /y:1080 /r:60

.\resochan.exe [<display>] /x <width> /y <height> [/r <refreshrate>]
.\resochan.exe /w 1920 /y 1080
.\resochan.exe /w 1920 /y 1080 /r 60
.\resochan.exe 2 /w 1920 /y 1080
.\resochan.exe 2 /w 1920 /y 1080 /r 60
.\resochan.exe \\.\DISPLAY2 /w 1920 /y 1080
.\resochan.exe \\.\DISPLAY2 /w 1920 /y 1080 /r 60
```
🔎 If no display is specified, the resolution will be applied to the display where your mouse cursor is currently located.

### List all connected displays
```pwsh
.\resochan.exe --list
```

### Show all modes for a display
```pwsh
.\resochan.exe --modes <display>
.\resochan.exe --modes 1
.\resochan.exe --modes \\.\DISPLAY1
```

### Test if a mode is supported without applying
```pwsh
.\resochan.exe --test <display> <width> <height> [<refreshrate>]
.\resochan.exe --test 1 2560 1440
.\resochan.exe --test 1 2560 1440 144
.\resochan.exe --test \\.\DISPLAY1 2560 1440
.\resochan.exe --test \\.\DISPLAY1 2560 1440 144
```

---

## System Requirements
 * Windows 10 or above
 * [Visual C++ 2015-2022 Redistributable](https://learn.microsoft.com/ko-kr/cpp/windows/latest-supported-vc-redist?view=msvc-170)

---

## 🔧 Build Instructions
### Requirements
 * Windows SDK
 * Visual Studio 2022 or later
 * C++20 or newer

### Build
Open ResoChan.sln in Visual Studio and build in either Release x64 or Debug x64 configuration.

---

## 📦 Deployment
 * Portable .exe, no installation required
 * Does not require administrative privileges
 * Ideal for scripts or one-click resolution switching

---

## 📄 License
GPL 3.0 (include Reso-chan Character and Image)

---

## 👤 Author
© 2025 HiSkyZen.

---
