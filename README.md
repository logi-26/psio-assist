# Psio-Assist-EX (Qt Version)

A Qt/C++ graphical application for preparing PlayStation 1 BIN/CUE games for use with the PSIO device.

Works on **Linux, macOS, and Windows**.

---

![image](https://github.com/user-attachments/assets/3702df62-7b1a-4af7-ac10-02ed044db05c)


---

## Features

- User-friendly graphical interface
- Support for Windows, Linux, and macOS
- ROM management
- Game transfer to PSIO

## Download

You can download the latest version of PSIO Assist directly from GitHub Actions artifacts:

1. Go to the [Actions page](https://github.com/gabrielBitts/psio-assist-ex/actions) of the repository
2. Click on the most recent run of the "Build for Windows, Linux and macOS" workflow
3. Scroll down to the "Artifacts" section at the bottom of the page
4. Download the artifact corresponding to your operating system:
   - Windows: `psio-assist-windows`
   - Linux: `psio-assist-linux`
   - macOS: `psio-assist-macos-app` (App Bundle) or `psio-assist-macos-dmg` (Disk image)

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
