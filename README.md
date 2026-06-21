# Fysik Project - Flight Simulation

This project is a real-time 3D flight simulation built with **C++** and **raylib**. It simulates realistic aircraft physics, including atmospheric effects, wind, thrust, lift, drag, and control surfaces. The simulation features a dynamic HUD, a compass, and a 3D environment with a textured ground and an aircraft model.

---

## Features

- **Real-time 3D Rendering** using raylib's OpenGL backend.
- **Physics Simulation** with forces: thrust, lift, drag, and weight.
- **Atmospheric Model** with altitude-based temperature, pressure, and density.
- **Dynamic Wind System** using Monte Carlo methods for realistic gusts and direction changes.
- **Interactive Controls**:
  - `W/S`: Throttle up/down
  - `Up/Down Arrows`: Pitch control
  - `A/D`: Roll control
  - `Q/E`: Yaw control
  - `F`: Toggle fullscreen
- **HUD** displaying speed, altitude, throttle, forces, and aircraft orientation.
- **Compass & Attitude Indicator** with heading, pitch, and roll information.
- **3D Aircraft Model** with textured components.
- **Engine Sound** with throttle-based volume and altitude-dependent sound switching.
- **Wind Vector Visualization** on the compass.

---

## Technologies Used

- **C++20**
- **raylib 6.0** for graphics, input, and audio
- **raymath** for vector and matrix math
- **stb_image** for texture loading
- **Custom physics engine** with Monte Carlo wind simulation

---

## Getting Started

### Prerequisites

- Windows (with Visual Studio 2022 or newer)
- [raylib 6.0](https://www.raylib.com/) (included in the project)
- C++ compiler with C++20 support

### Installation

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/fysik_project.git
