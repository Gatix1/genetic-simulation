#include "ui.h"
#include <string>
#include <fstream>

#include "instructions.h"
UI::UI() {}

UI::~UI() {
    for (auto& bot_info : loaded_bots) {
        delete bot_info.bot;
    }
    loaded_bots.clear();
}

bool UI::isPaused() const {
    return is_paused;
}

int UI::getViewMode() const {
    return current_view_mode;
}

Bot* UI::getOrganismRoot() const {
    return organism_root;
}

int UI::getSpeedDivisor() const {
    return speed_divisor;
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
        selected_loaded_bot = nullptr;
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
            Vector2 target_pos = {(float)grid_x, (float)grid_y};

            if (selected_loaded_bot != nullptr) {
                if (world.getBotAt(target_pos) == nullptr) {
                    Bot* new_bot = new Bot(*selected_loaded_bot);
                    new_bot->setPosition(target_pos);
                    world.addBot(new_bot);
                }
            } else {
                selected_bot = world.getBotAt(target_pos);
                organism_root = selected_bot;
            }
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
        ImGui::Dummy(ImVec2(10.0f, 0.0f));
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Spawn Bots")) {
                show_spawn_bots_modal = true;
            }
            ImGui::EndMenu();
        }
        ImGui::Dummy(ImVec2(10.0f, 0.0f));
        if (ImGui::BeginMenu("Speed")) {
            if (ImGui::MenuItem("Original", NULL, speed_divisor == 1)) speed_divisor = 1;
            if (ImGui::MenuItem("1/2", NULL, speed_divisor == 2)) speed_divisor = 2;
            if (ImGui::MenuItem("1/4", NULL, speed_divisor == 4)) speed_divisor = 4;
            if (ImGui::MenuItem("1/12", NULL, speed_divisor == 12)) speed_divisor = 12;
            ImGui::EndMenu();
        }
        ImGui::Dummy(ImVec2(10.0f, 0.0f));
        if (ImGui::BeginMenu("Bot")) {
            if (ImGui::MenuItem("Save Bot", NULL, false, selected_bot != nullptr)) {
                show_save_bot_modal = true;
            }
            if (ImGui::MenuItem("Load Bot")) {
                show_load_bot_modal = true;
            }
            ImGui::EndMenu();
        }
        ImGui::Dummy(ImVec2(10.0f, 0.0f));
        if (ImGui::BeginMenu("Loaded Bots")) {
            for (const auto& bot_info : loaded_bots) {
                if (ImGui::MenuItem(bot_info.filename.c_str(), NULL, selected_loaded_bot == bot_info.bot)) {
                    selected_loaded_bot = bot_info.bot;
                    selected_bot = nullptr;
                    organism_root = nullptr;
                }
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

    // 5. Save Bot Modal
    if (show_save_bot_modal) {
        ImGui::OpenPopup("Save Bot");
        show_save_bot_modal = false;
    }
    if (ImGui::BeginPopupModal("Save Bot", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Filename", bot_filename_buffer, IM_ARRAYSIZE(bot_filename_buffer));
        if (ImGui::Button("Save", ImVec2(0, 0))) {
            if (selected_bot) {
                std::ofstream out(bot_filename_buffer, std::ios::binary);
                if (out.is_open()) {
                    selected_bot->serialize(out);
                    out.close();
                }
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(0, 0))) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }

    // 6. Load Bot Modal
    if (show_load_bot_modal) {
        ImGui::OpenPopup("Load Bot");
        show_load_bot_modal = false;
    }
    if (ImGui::BeginPopupModal("Load Bot", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::InputText("Filename", bot_filename_buffer, IM_ARRAYSIZE(bot_filename_buffer));
        if (ImGui::Button("Load", ImVec2(0, 0))) {
            std::ifstream in(bot_filename_buffer, std::ios::binary);
            if (in.is_open()) {
                Bot* bot = new Bot();
                bot->deserialize(in);
                loaded_bots.push_back({std::string(bot_filename_buffer), bot});
                in.close();
            }
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
    Bot* inspector_bot = selected_bot ? selected_bot : selected_loaded_bot;
    if (inspector_bot != nullptr) {
        if (selected_bot) ImGui::TextColored(ImVec4(1, 1, 0, 1), "SELECTED BOT");
        else ImGui::TextColored(ImVec4(0, 1, 1, 1), "LOADED BOT (Placement Mode)");
        ImGui::Separator();

        if (inspector_bot->isOrganic) {
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Status: Organic Matter");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Alive");
        }
        
        ImGui::Text("Energy: %d", inspector_bot->getEnergy());
        
        Color c = inspector_bot->getColor();
        float color[4] = { c.r/255.0f, c.g/255.0f, c.b/255.0f, 1.0f };
        ImGui::ColorEdit3("Color", color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoPicker);

        ImGui::Text("Age: %d", inspector_bot->getAge());

        ImGui::Separator();
        ImGui::Text("Memory Stack");
        if (ImGui::BeginChild("MemoryStack", ImVec2(0, 100), true)) {
            const std::stack<unsigned int> memory = inspector_bot->getMemory();
            // Display from top to bottom
            for (int i = 0; i < (int)memory.size(); ++i) {
                ImGui::Text("%02d: %u", (int)memory.size() - 1 - i, memory.top());
            }
        }
        ImGui::EndChild();

        ImGui::Separator();
        ImGui::Text("Genome");
        if (ImGui::BeginChild("GenomeView", ImVec2(0, 150), true)) {
            const std::vector<unsigned int>& genome = inspector_bot->getGenome();
            unsigned int pc = inspector_bot->getPC();
            for (size_t i = 0; i < genome.size(); ++i) {
                unsigned int val = genome[i];
                const char* instr = "JUMP";
                switch(val) {
                    case MOVE: instr = "MOVE"; break;
                    case TURN: instr = "TURN"; break;
                    case LOOK: instr = "LOOK"; break;
                    case ATTACK: instr = "ATTACK"; break;
                    case PHOTOSYNTHIZE: instr = "PHOTO"; break;
                    case CHECK_RELATIVE: instr = "RELAT"; break;
                    case SHARE_ENERGY: instr = "SHARE"; break;
                    case CONSUME_ORGANIC: instr = "EAT"; break;
                    case REPRODUCE: instr = "REPRO"; break;
                    case JUMP_UNCONDITIONAL: instr = "JUMP_U"; break;
                    case CHECK_BIOME: instr = "BIOME"; break;
                    case CHECK_X: instr = "CH_X"; break;
                    case CHECK_Y: instr = "CH_Y"; break;
                    case CHECK_ENERGY: instr = "CH_NRG"; break;
                    case CHECK_AGE: instr = "CH_AGE"; break;
                    case JUMP_IF_EQUAL: instr = "JE"; break;
                    case JUMP_IF_NOT_EQUAL: instr = "JNE"; break;
                    case JUMP_IF_GREATER: instr = "JG"; break;
                }

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
