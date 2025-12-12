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
                show_save_world_modal = true;
            }
            if (ImGui::MenuItem("Load")) {
                show_load_world_modal = true;
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
        ImGui::Text("Enter a seed (text or number). Leave empty for random.");
        ImGui::InputText("Seed", seed_buffer, IM_ARRAYSIZE(seed_buffer));
        ImGui::InputInt("Initial Bots", &initial_bots_count);
        ImGui::Separator();

        if (ImGui::Button("Create", ImVec2(0, 0))) {
            unsigned int final_seed;
            if (strlen(seed_buffer) == 0) {
                final_seed = (unsigned int)time(NULL);
                snprintf(seed_buffer, sizeof(seed_buffer), "%u", final_seed);
            } else {
                char* p_end;
                unsigned long long parsed_seed = strtoull(seed_buffer, &p_end, 10);
                if (*p_end == '\0' && p_end != seed_buffer) {
                    final_seed = (unsigned int)parsed_seed;
                } else {
                    final_seed = 0;
                    for (const char* p = seed_buffer; *p; ++p) final_seed = 31 * final_seed + *p;
                }
            }
            world.newWorld(final_seed, initial_bots_count);
            ImGui::CloseCurrentPopup();
        }
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
        if (ImGui::Button("Spawn", ImVec2(0, 0))) {
            world.spawnInitialBots(bots_to_spawn_count);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(0, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    // 3. Save World Modal
    if (show_save_world_modal) {
        ImGui::OpenPopup("Save World");
        show_save_world_modal = false;
    }
    if (ImGui::BeginPopupModal("Save World", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Filename", save_filename_buffer, IM_ARRAYSIZE(save_filename_buffer));
        if (ImGui::Button("Save", ImVec2(0, 0))) { 
            world.saveWorld(save_filename_buffer);
            ImGui::CloseCurrentPopup(); 
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(0, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    // 4. Load World Modal
    if (show_load_world_modal) {
        ImGui::OpenPopup("Load World");
        show_load_world_modal = false;
    }
    if (ImGui::BeginPopupModal("Load World", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Filename", save_filename_buffer, IM_ARRAYSIZE(save_filename_buffer));
        if (ImGui::Button("Load", ImVec2(0, 0))) { 
            world.loadWorld(save_filename_buffer);
            selected_bot = nullptr;
            organism_root = nullptr;
            snprintf(seed_buffer, sizeof(seed_buffer), "%u", world.getSeed());
            ImGui::CloseCurrentPopup(); 
        }
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

        ImGui::Text("Age: %d", selected_bot->getAge());

        ImGui::Separator();
        ImGui::Text("Memory Stack");
        if (ImGui::BeginChild("MemoryStack", ImVec2(0, 100), true)) {
            const std::stack<unsigned int> memory = selected_bot->getMemory();
            // Display from top to bottom
            for (int i = 0; i < (int)memory.size(); ++i) {
                ImGui::Text("%02d: %u", (int)memory.size() - 1 - i, memory.top());
            }
        }
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::Text("Genome");
        if (ImGui::BeginChild("GenomeView", ImVec2(0, 150), true)) {
            const std::vector<unsigned int>& genome = selected_bot->getGenome();
            unsigned int pc = selected_bot->getPC();
            for (size_t i = 0; i < genome.size(); ++i) {
                unsigned int val = genome[i];
                const char* instr = "JUMP";
                if (val == 0) instr = "MOVE";
                else if (val == 1) instr = "TURN";
                else if (val == 2) instr = "LOOK";
                else if (val == 3) instr = "ATTACK";
                else if (val == 4) instr = "PHOTO";
                else if (val == 5) instr = "RELAT";
                else if (val == 6) instr = "SHARE";
                else if (val == 7) instr = "EAT";
                else if (val == 8) instr = "REPRO";
                else if (val == 10) instr = "BIOME";
                else if (val == 11) instr = "CH_X";
                else if (val == 12) instr = "CH_Y";
                else if (val == 13) instr = "CH_NRG";
                else if (val == 14) instr = "CH_AGE";
                else if (val == 15) instr = "JE";
                else if (val == 16) instr = "JNE";
                else if (val == 17) instr = "JG";

                if (i == pc) {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "> %02zu: %s (%u)", i, instr, val);
                } else {
                    ImGui::Text("  %02zu: %s (%u)", i, instr, val);
                }
            }
        }
        ImGui::EndChild();
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
