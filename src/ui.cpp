#include "ui.h"
#include <string>

UI::UI() {
    const int screenHeight = WORLD_HEIGHT * CELL_SIZE + BOTTOM_PANEL_HEIGHT;
    button1_rect = { (float)WORLD_WIDTH * CELL_SIZE - 150, (float)screenHeight - 35, 30, 30 };
    button2_rect = { (float)WORLD_WIDTH * CELL_SIZE - 110, (float)screenHeight - 35, 30, 30 };
    button3_rect = { (float)WORLD_WIDTH * CELL_SIZE - 70, (float)screenHeight - 35, 30, 30 };
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

void UI::handleInput(World& world) {
    // Keyboard input
    if (IsKeyPressed(KEY_SPACE)) {
        is_paused = !is_paused;
    }
    if (IsKeyPressed(KEY_ONE)) current_view_mode = 1;
    if (IsKeyPressed(KEY_TWO)) current_view_mode = 2;
    if (IsKeyPressed(KEY_THREE)) current_view_mode = 3;

    // Mouse input
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        selected_bot = nullptr;
        organism_root = nullptr;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse_pos = GetMousePosition();
        if (CheckCollisionPointRec(mouse_pos, button1_rect)) {
            current_view_mode = 1;
        } else if (CheckCollisionPointRec(mouse_pos, button2_rect)) {
            current_view_mode = 2;
        } else if (CheckCollisionPointRec(mouse_pos, button3_rect)) {
            current_view_mode = 3;
        } else if (mouse_pos.x < WORLD_WIDTH * CELL_SIZE && mouse_pos.y < WORLD_HEIGHT * CELL_SIZE) {
            // Click was in the world, not on a UI button
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

void UI::draw(const World& world) {
    if (is_paused) {
        DrawText("PAUSED", 10, 10, 20, RED);
    }

    // Draw selection border if a bot is selected
    if (selected_bot != nullptr) {
        Vector2 pos = selected_bot->getPosition();
        DrawRectangleLinesEx({pos.x * CELL_SIZE, pos.y * CELL_SIZE, (float)CELL_SIZE, (float)CELL_SIZE}, 3, BLACK);
    }

    _drawBottomPanel(world);
    _drawSidePanel();
}

void UI::_drawBottomPanel(const World& world) const {
    DrawRectangle(0, WORLD_HEIGHT * CELL_SIZE, WORLD_WIDTH * CELL_SIZE, BOTTOM_PANEL_HEIGHT, BLACK);
    std::string bot_count_text = "Bots: " + std::to_string(world.getBotsSize());
    std::string step_count_text = "Step: " + std::to_string(world.getStepCount());
    std::string fps_text = "FPS: " + std::to_string(GetFPS());
    DrawText(bot_count_text.c_str(), 10, WORLD_HEIGHT * CELL_SIZE + 15, 20, RAYWHITE);
    DrawText(step_count_text.c_str(), 200, WORLD_HEIGHT * CELL_SIZE + 15, 20, RAYWHITE);
    DrawText(fps_text.c_str(), 450, WORLD_HEIGHT * CELL_SIZE + 15, 20, RAYWHITE);

    // View Mode UI
    DrawText("View Mode:", WORLD_WIDTH * CELL_SIZE - 280, WORLD_HEIGHT * CELL_SIZE + 15, 20, RAYWHITE);
    DrawRectangleRec(button1_rect, current_view_mode == 1 ? DARKBLUE : LIGHTGRAY); DrawText("1", button1_rect.x + 10, button1_rect.y + 5, 20, BLACK);
    DrawRectangleRec(button2_rect, current_view_mode == 2 ? DARKBLUE : LIGHTGRAY); DrawText("2", button2_rect.x + 10, button2_rect.y + 5, 20, BLACK);
    DrawRectangleRec(button3_rect, current_view_mode == 3 ? DARKBLUE : LIGHTGRAY); DrawText("3", button3_rect.x + 10, button3_rect.y + 5, 20, BLACK);
}

void UI::_drawSidePanel() const {
    const int screenHeight = WORLD_HEIGHT * CELL_SIZE + BOTTOM_PANEL_HEIGHT;
    DrawRectangle(WORLD_WIDTH * CELL_SIZE, 0, SIDE_PANEL_WIDTH, screenHeight, BLACK);
    if (selected_bot != nullptr) {
        DrawText("SELECTED BOT", WORLD_WIDTH * CELL_SIZE + 10, 10, 20, YELLOW);

        // Determine status
        std::string status_text = "Status: ";
        Color status_color = RED;
        if (selected_bot->isOrganic) {
            status_text += "Organic";
            status_color = GRAY;
        } else {
            status_text += "Alive";
            status_color = GREEN;
        }
        DrawText(status_text.c_str(), WORLD_WIDTH * CELL_SIZE + 10, 40, 20, status_color);
        std::string energy_text = "Energy: " + std::to_string(selected_bot->getEnergy());
        DrawText(energy_text.c_str(), WORLD_WIDTH * CELL_SIZE + 10, 70, 20, RAYWHITE);
    }
}
