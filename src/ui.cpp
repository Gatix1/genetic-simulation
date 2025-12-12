#include "ui.h"
#include <string>

UI::UI() {}

bool UI::isPaused() const {
    return is_paused;
}

int UI::getViewMode() const {
    return current_view_mode;
}

Bot* UI::getOrganismRoot() const {
    return organism_root;
}

void UI::handleInput(World& world) {
    // Keyboard input
    // We check WantCaptureKeyboard to prevent triggering hotkeys while typing in InputText or navigating menus/modals.
    if (!ImGui::GetIO().WantCaptureKeyboard) {
        if (IsKeyPressed(KEY_SPACE)) is_paused = !is_paused;
        if (IsKeyPressed(KEY_ONE)) current_view_mode = 1;
        if (IsKeyPressed(KEY_TWO)) current_view_mode = 2;
        if (IsKeyPressed(KEY_THREE)) current_view_mode = 3;
    }

    // Mouse input
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        selected_bot = nullptr;
        organism_root = nullptr;
    }

    // ImGui::GetIO().WantCaptureMouse is true if the mouse is hovering over an ImGui window.
    // We only want to process world clicks (selecting bots) if the mouse is NOT interacting with the UI.
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !ImGui::GetIO().WantCaptureMouse) {
        Vector2 mouse_pos = GetMousePosition();
        mouse_pos.y -= TOP_PANEL_HEIGHT; // Adjust for top panel
        
        // Check if click is within the world bounds
        if (mouse_pos.x >= 0 && mouse_pos.x < WORLD_WIDTH * CELL_SIZE && mouse_pos.y >= 0 && mouse_pos.y < WORLD_HEIGHT * CELL_SIZE) {
            int grid_x = mouse_pos.x / CELL_SIZE;
            int grid_y = mouse_pos.y / CELL_SIZE;
            selected_bot = world.getBotAt({(float)grid_x, (float)grid_y});

            organism_root = selected_bot;
        }
    }

    // State update
    if (organism_root != nullptr && organism_root->is_dead) {
        selected_bot = nullptr;
        organism_root = nullptr;
    }
}

void UI::drawWorldOverlay() const {
    // World Overlay (Raylib)
    // We draw the selection box directly in the world using Raylib functions because
    // it needs to be aligned with the grid, not the UI layer.
    if (selected_bot != nullptr) {
        Vector2 pos = selected_bot->getPosition();
        DrawRectangleLinesEx({pos.x * CELL_SIZE, pos.y * CELL_SIZE, (float)CELL_SIZE, (float)CELL_SIZE}, 3, BLACK);
    }
}

void UI::drawPanels(World& world) {
    // --- Main Menu Bar ---
    // Make menu bar transparent and borderless
    if (ImGui::BeginMainMenuBar()) {
        ImGui::Dummy(ImVec2(10.0f, 0.0f)); // Add left margin
        ImGui::SameLine();

        if (ImGui::BeginMenu("World")) {
            if (ImGui::MenuItem("New World")) {
                show_new_world_modal = true;
            }
            if (ImGui::MenuItem("Save")) {
                // Implementation to come
            }
            if (ImGui::MenuItem("Load")) {
                // Implementation to come
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Spawn Bots")) {
                show_spawn_bots_modal = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // --- Modals (Floating Windows) ---
    
    // 1. New World Modal
    if (show_new_world_modal) {
        ImGui::OpenPopup("New World Options");
        show_new_world_modal = false;
    }
    if (ImGui::BeginPopupModal("New World Options", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter a seed for the new world generation:");
        ImGui::InputText("Seed", seed_buffer, IM_ARRAYSIZE(seed_buffer));
        ImGui::Separator();

        if (ImGui::Button("New World", ImVec2(0, 0))) { ImGui::CloseCurrentPopup(); /* Logic to come */ }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(0, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    // 2. Spawn Bots Modal
    if (show_spawn_bots_modal) {
        ImGui::OpenPopup("Spawn Bots Options");
        show_spawn_bots_modal = false;
    }
    if (ImGui::BeginPopupModal("Spawn Bots Options", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputInt("Amount", &bots_to_spawn_count);
        if (ImGui::Button("Spawn", ImVec2(0, 0))) { ImGui::CloseCurrentPopup(); /* Logic to come */ }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(0, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    // Side Panel (ImGui)
    ImGui::SetNextWindowPos(ImVec2(WORLD_WIDTH * CELL_SIZE, TOP_PANEL_HEIGHT));
    ImGui::SetNextWindowSize(ImVec2(SIDE_PANEL_WIDTH, WORLD_HEIGHT * CELL_SIZE));
    ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
    // Dynamic content: The UI changes immediately based on whether a bot is selected.
    if (selected_bot != nullptr) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "SELECTED BOT");
        ImGui::Separator();

        if (selected_bot->isOrganic) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Status: Organic Matter");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Alive");
        }
        
        ImGui::Text("Energy: %d", selected_bot->getEnergy());
        
        Color c = selected_bot->getColor();
        float color[4] = { c.r/255.0f, c.g/255.0f, c.b/255.0f, 1.0f };
        ImGui::ColorEdit3("Color", color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);
    } else {
        ImGui::TextWrapped("Click on a bot in the grid to inspect it.");
    }
    ImGui::End();

    // Bottom Panel (ImGui)
    ImGui::SetNextWindowPos(ImVec2(0, TOP_PANEL_HEIGHT + WORLD_HEIGHT * CELL_SIZE));
    ImGui::SetNextWindowSize(ImVec2(WORLD_WIDTH * CELL_SIZE + SIDE_PANEL_WIDTH, BOTTOM_PANEL_HEIGHT));
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    
    // Stats
    ImGui::Text("Bots: %d", world.getBotsSize());
    ImGui::SameLine(0.0f, 30.0f);
    ImGui::Text("Step: %lld", world.getStepCount());
    ImGui::SameLine(0.0f, 30.0f);
    ImGui::Text("FPS: %d", GetFPS());

    ImGui::SameLine(0.0f, 60.0f);
    // ImGui::Button returns true only on the frame it is clicked.
    if (ImGui::Button(is_paused ? "Resume (Space)" : "Pause (Space)")) {
        is_paused = !is_paused;
    }

    // Radio buttons for switching view modes. They update 'current_view_mode' directly.
    ImGui::SameLine(0.0f, 60.0f);
    ImGui::Text("View:");
    ImGui::SameLine();
    if (ImGui::RadioButton("Nutrition (1)", current_view_mode == 1)) current_view_mode = 1;
    ImGui::SameLine();
    if (ImGui::RadioButton("Energy (2)", current_view_mode == 2)) current_view_mode = 2;
    ImGui::SameLine();
    if (ImGui::RadioButton("Species (3)", current_view_mode == 3)) current_view_mode = 3;

    ImGui::End();
}
