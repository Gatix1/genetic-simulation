#pragma once
#include "raylib.h"
#include "world.h"
#include "imgui.h"
#include "config.h"

// The UI class handles the overlay interface using Dear ImGui.
// It bridges the gap between the simulation state (World/Bot) and the user.
// Note: Input handling here is for selecting entities in the world; 
// ImGui handles its own input (buttons, sliders) internally within the draw() method.
class UI {
public:
    UI();
    ~UI();
    void handleInput(World& world);
    void drawWorldOverlay() const;
    void drawPanels(const World& world);
    void drawPanels(World& world);

    bool isPaused() const;
    int getViewMode() const;
    Bot* getOrganismRoot() const;
    int getSpeedDivisor() const;

    bool isScanningRelatives() const;
    const std::vector<Bot*>& getHighlightedRelatives() const;

private:
    // State
    bool is_paused = false;
    int current_view_mode = 2; // 1: Nutrition, 2: Species Color
    Bot* selected_bot = nullptr;
    Bot* organism_root = nullptr;
    int speed_divisor = 1;

    // Relative scanning state
    bool is_scanning_relatives = false;
    std::vector<Bot*> highlighted_relatives;
    Bot* scan_origin_bot = nullptr;

    // Top panel state
    char seed_buffer[128] = "";
    int bots_to_spawn_count = 100;
    int initial_bots_count = 10000;

    // Modal state
    bool show_new_world_modal = false;
    bool show_spawn_bots_modal = false;
    bool show_save_world_modal = false;
    bool show_load_world_modal = false;
    char save_filename_buffer[128] = "world.save";

    // Bot management state
    struct LoadedBotInfo {
        std::string filename;
        Bot* bot;
    };
    std::vector<LoadedBotInfo> loaded_bots;
    Bot* selected_loaded_bot = nullptr;
    bool show_save_bot_modal = false;
    bool show_load_bot_modal = false;
    char bot_filename_buffer[128] = "bot.save";
};
