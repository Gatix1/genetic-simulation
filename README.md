# Genetic Algorithm Simulation

This is a 2D grid based simulation of evolution through the genetic algorithm.
There are "bots" which have a genome, which is a program of N commands which loops.
On the start of the program all genomes are random, those bots who succeed to survive
and reproduce pass their genes with a slight chance of mutation which allows for evolution-like
behavior.

## Building from Source

This project uses CMake and depends on the Raylib library, which is included as a git submodule.

### Prerequisites

- A C++ compiler (like GCC, Clang, or MSVC)
- CMake (version 3.15 or higher)
- Git

### Compilation Steps

1.  **Clone the repository with submodules:**
    ```bash
    git clone --recurse-submodules https://github.com/Gatix1/genetic-simulation.git
    cd genetic-simulation
    ```
    If you've already cloned the repository without the submodules, you can initialize them with:
    ```bash
    git submodule update --init --recursive
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Configure the project with CMake:**
    ```bash
    cmake ..
    ```

4.  **Build the project:**
    ```bash
    cmake --build .
    ```
    Alternatively, on Linux or macOS, you can just run `make`.

5.  **Run the executable:**
    The compiled executable will be in the `build` directory.
    ```bash
    ./main
    ```

## Controls

- **`Space`**: Pause / Resume the simulation.
- **`1`**: Switch to Nutrition view mode.
- **`2`**: Switch to Energy view mode.
- **`3`**: Switch to Species Color view mode.
- **`Left Mouse Button`**: Select a bot to view its details in the side panel.
- **`Right Mouse Button`**: Deselect the current bot.

## License

Apache License 2.0. 
Check LICENSE file for further information.
