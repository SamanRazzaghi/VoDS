A pre-built project ready to run. If you want to build it yourself, follow the instructions below!

---

## Build Instructions

### 🐧 Linux

1. Make sure you have `cmake`, `make`, and `mingw` installed.

2. Make the build script executable:

   ```bash
   chmod +x buildall.sh
   
3. Run the build script:
   ```bash
   ./buildall.sh
This will build the project for both Linux and Windows (using `mingw`).

---

### 🪟 Windows

1. Ensure **MSVC (Visual Studio)** and **CMake** are installed.

2. Open Command Prompt or Windows Terminal and run:
   mkdir build-msvc
   cd build-msvc
   cmake ..
   cmake --build .
   
3. Find the executable in the `Debug` folder inside `build-msvc`.

---
