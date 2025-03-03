# Psio-Assist-EX (Qt Version)

A Qt/C++ graphical application for preparing PlayStation 1 BIN/CUE games for use with the PSIO device.

Works on **Linux, macOS, and Windows**.

---

## ![Application Screenshot](insert_image_link_here)

---

## Features

This application organizes and standardizes PlayStation 1 games into a format acceptable by the PSIO device. It performs the following tasks:

- Merges **multi-BIN** games into a single BIN file.
- Converts **CUE** files to the **CU2** format.
- Works in **batch mode**, processing all BIN/CUE files in the selected directory and its subdirectories.
- Ensures that **game names**:
  - Do not exceed **60 characters**.
  - Do not contain **dots or slashes**.
- Generates the **MULTIDISC.LST** file for multi-disc games.
- **Does not modify the original BIN/CUE files**.
- **Downloads cover images** for games that do not have them in their directories.

---

## Dependencies

This project requires:

- **Qt 6.5** or later
- **CMake 3.5** or later
- **C++ compiler** with **C++17** support

### Installing Dependencies

#### Ubuntu/Debian
```bash
sudo apt update && sudo apt install qt6-base-dev cmake g++
```

#### Fedora
```bash
sudo dnf install qt6-qtbase-devel cmake g++
```

#### macOS (using Homebrew)
```bash
brew install qt cmake
```

#### Windows
- Install **Qt** using the [Qt Online Installer](https://www.qt.io/download)
- Install **CMake** from the [official website](https://cmake.org/download/)
- Install **Visual Studio** or **MinGW**

---

## Compilation

Clone the repository:
```bash
git clone https://github.com/gabrielBitts/psio-assist-ex.git
cd psio-assist-ex
```

Compile the project:
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)  # Linux/macOS
cmake --build .   # Windows (using CMake)
```

---

## Usage

### 1. Run the compiled application:
```bash
./Psio-Assist-EX  # Linux/macOS
Psio-Assist-EX.exe  # Windows
```

### 2. Using the interface:
1. Click the **Browse** button and select the directory containing your BIN/CUE files.
2. Select the desired **processing options**.
3. Click **Process** to start the operation.
4. The progress bar will indicate the task progress.

---

## License

This project is licensed under **GPL-3.0**.

---

## Credits

- **Qt/C++ Interface:** Gabriel Bitts
- **Based on the original project:** [PSIO-Assist](https://github.com/gabrielBitts/psio-assist-ex)

---
