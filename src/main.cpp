#include "raylib.h"
#include "rlImGui.h"
#include "rlgl.h"
#include "rlgl.h"
#include "world.h"
#include "config.h"
#include "ui.h"
#include <random>
#include <string>
#include <ctime>

int main(void)
{
    const int screenWidth = WORLD_WIDTH * CELL_SIZE + SIDE_PANEL_WIDTH;
    const int screenHeight = TOP_PANEL_HEIGHT + WORLD_HEIGHT * CELL_SIZE + BOTTOM_PANEL_HEIGHT;

    World world = World();
    world.newWorld((unsigned int)time(NULL), 10000);

    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(screenWidth, screenHeight, "Simulation");
    rlImGuiSetup(true); // Initialize ImGui with dark mode

    // Increase the global UI scale for better readability.
    ImGui::GetIO().FontGlobalScale = 1.0f;

    // Darken the modal window dimming background
    ImGui::GetStyle().Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);

    SetExitKey(KEY_NULL);

    SetTargetFPS(0);
    UI ui;
    int frame_counter = 0;

    // Main loop
    while (!WindowShouldClose())
    {
        // --- Input Handling ---
        ui.handleInput(world);

        // --- State Update ---
        if (!ui.isPaused()) {
            if (frame_counter % ui.getSpeedDivisor() == 0) {
                world.process();
                ui.update(); // Check for dead selected bot after processing
            }
        }
        frame_counter = (frame_counter + 1) % 12;

        // --- Drawing ---
        BeginDrawing();
            ClearBackground(BG_COLOR);
            // --- Simulation & UI ---
            rlPushMatrix();
            rlTranslatef(0, (float)TOP_PANEL_HEIGHT, 0);
            world.render(ui.getViewMode(), ui.getOrganismRoot(), ui.getHighlightedRelatives());
            ui.drawWorldOverlay();
            rlPopMatrix();
            
            rlImGuiBegin();
            ui.drawPanels(world);
            rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    CloseWindow(); 

    return 0;
}
