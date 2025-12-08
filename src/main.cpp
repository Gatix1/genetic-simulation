#include "raylib.h"
#include "world.h"
#include "config.h"
#include "ui.h"
#include <random>
#include <string>
#include <ctime>

int main(void)
{
    const int screenWidth = WORLD_WIDTH * CELL_SIZE + SIDE_PANEL_WIDTH; 
    const int screenHeight = WORLD_HEIGHT * CELL_SIZE + BOTTOM_PANEL_HEIGHT;

    SetRandomSeed((unsigned int)time(NULL));

    World world = World();
    world.spawnInitialBots(1000);

    InitWindow(screenWidth, screenHeight, "Simulation");

    SetTargetFPS(300);

    UI ui;

    // Main loop
    while (!WindowShouldClose())
    {
        // --- Input Handling ---
        ui.handleInput(world);

        // --- State Update ---
        if (!ui.isPaused()) {
            world.process();
        }

        // --- Drawing ---
        BeginDrawing();
            ClearBackground(BG_COLOR);

            // --- Simulation & UI ---
            world.render(ui.getViewMode(), ui.getOrganismRoot());
            ui.draw(world);

        EndDrawing();
    }

    CloseWindow(); 

    return 0;
}
