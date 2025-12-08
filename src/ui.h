#pragma once
#include "raylib.h"
#include "world.h"
#include "config.h"

class UI {
public:
    UI();
    void handleInput(World& world);
    void draw(const World& world);

    bool isPaused() const;
    int getViewMode() const;
    Bot* getOrganismRoot() const;

private:
    // State
    bool is_paused = false;
    int current_view_mode = 3; // 1: Nutrition, 2: Energy, 3: Species Color
    Bot* selected_bot = nullptr;
    Bot* organism_root = nullptr;

    // UI Element Rectangles
    Rectangle button1_rect;
    Rectangle button2_rect;
    Rectangle button3_rect;

    void _drawSidePanel() const;
    void _drawBottomPanel(const World& world) const;
};
