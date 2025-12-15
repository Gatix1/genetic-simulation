#include <world.h>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include "config.h"

World::World() : World(WORLD_WIDTH, WORLD_HEIGHT) {}

World::World(int width, int height) : world_width(width), world_height(height) {
    grid.resize(width);
    for (int i = 0; i < width; ++i) {
        grid[i].resize(height, nullptr);
    }
}

void World::newWorld(unsigned int seed, int initial_bot_count) {
    clear();
    this->seed = seed;
    SetRandomSeed(seed);
    spawnInitialBots(initial_bot_count);
}

void World::spawnInitialBots(int count) {
    for (int i = 0; i < count; i++) {
        Bot* bot = new Bot();
        Vector2 spawn_pos = {-1, -1};
        int attempts = 0;
        const int max_attempts = world_width * world_height;

        // Find a random empty cell for the new bot
        do {
            spawn_pos = {(float)GetRandomValue(0, world_width - 1), (float)GetRandomValue(0, world_height - 1)};
            if (attempts++ > max_attempts) {
                delete bot; // Clean up memory
                throw std::runtime_error("Could not find an empty cell to spawn a new bot.");
            }
        } while (getBotAt(spawn_pos) != nullptr);

        bot->setPosition(spawn_pos);
        this->addBot(bot);
    }
}

void World::addBot(Bot *bot_ptr) {
    this->bots.push_back(bot_ptr);
    this->grid[(int)bot_ptr->getPosition().x][(int)bot_ptr->getPosition().y] = bot_ptr;
}

void World::removeBot(Bot *bot_ptr) {
    bot_ptr->is_dead = true;
    this->grid[(int)bot_ptr->getPosition().x][(int)bot_ptr->getPosition().y] = nullptr;
}

void World::updateBotPosition(Bot* bot_ptr, Vector2 old_pos) {
    if (old_pos.x >= 0 && old_pos.x < world_width && old_pos.y >= 0 && old_pos.y < world_height)
        this->grid[(int)old_pos.x][(int)old_pos.y] = nullptr;
    if (bot_ptr->getPosition().x >= 0 && bot_ptr->getPosition().x < world_width && bot_ptr->getPosition().y >= 0 && bot_ptr->getPosition().y < world_height)
        this->grid[(int)bot_ptr->getPosition().x][(int)bot_ptr->getPosition().y] = bot_ptr;
}

const std::vector<Bot*>& World::getBots() const {
    return this->bots;
}

World::~World() {
    for (Bot* bot : bots) {
        delete bot;
    }
    // The vector will be cleared automatically when the World object is destroyed.
}

void World::render(int view_mode, Bot* selected_bot, const std::vector<Bot*>& relatives) {
    // --- Draw Biome Backgrounds ---
    if (world_width == WORLD_WIDTH && world_height == WORLD_HEIGHT) { // Only for main world
        DrawRectangle(0, 0, (world_width / 3) * CELL_SIZE, world_height * CELL_SIZE, {255, 200, 0, 40});
        DrawRectangle((world_width / 3) * CELL_SIZE, 0, (world_width / 3) * CELL_SIZE, world_height * CELL_SIZE, {0, 255, 100, 40});
        DrawRectangle((2 * world_width / 3) * CELL_SIZE, 0, (world_width / 3) * CELL_SIZE, world_height * CELL_SIZE, {0, 255, 255, 40});
    }

    bool highlight_mode = (selected_bot != nullptr);

    for (Bot* bot : this->bots) {
        bool is_relative = std::find(relatives.begin(), relatives.end(), bot) != relatives.end();
        bool is_selected = (bot == selected_bot);
        if (!highlight_mode || is_selected || is_relative) {
            bot->render(view_mode);
            if (is_relative) {
                DrawRectangleLinesEx({bot->getPosition().x * CELL_SIZE, bot->getPosition().y * CELL_SIZE, (float)CELL_SIZE, (float)CELL_SIZE}, 2, WHITE);
            }
        } else {
            bot->render(view_mode, (unsigned char)(255.0 * 0.2));
        }
    }

    // Draw grid
    for (int i = 0; i < world_width; i++) {
        DrawLineEx({float(i) * CELL_SIZE, 0},
                   {float(i) * CELL_SIZE, (float)world_height * CELL_SIZE},
                   GRID_THICKNESS, GRID_COLOR);
    }
    for (int i = 0; i < world_height; i++) {
        DrawLineEx({0, float(i) * CELL_SIZE},
                   {(float)world_width * CELL_SIZE, float(i) * CELL_SIZE},
                   GRID_THICKNESS, GRID_COLOR);
    }
}

void World::process() {
    this->step_count++;

    // Create a copy of the bots vector to iterate over, as the original
    // vector might be modified during the loop (bots being added or removed).
    std::vector<Bot*> bots_to_process = this->bots;
    for (Bot* bot : bots_to_process) {
        // A bot might have been marked as dead by another bot's action in this same frame.
        // If so, don't process it.
        if (!bot->is_dead) {
            bot->process(*this);
        }
    }

    // Second phase: clean up bots that were marked as dead during the processing phase.
    auto it = std::remove_if(this->bots.begin(), this->bots.end(), [](Bot* bot) {
        if (bot->is_dead) {
            // In the GenomeAnalyzer, we need to be careful not to delete the main sim_bot
            // while the analyzer might still reference it in the same frame.
            // The analyzer will handle the cleanup of sim_bot.
            if (bot->isOrganic || bot->getEnergy() > 0) delete bot;
            return true;
        }
        return false;
    });
    this->bots.erase(it, this->bots.end());
}

Bot* World::getBotAt(Vector2 position) {
    if (position.x < 0 || position.x >= world_width || position.y < 0 || position.y >= world_height) {
        return nullptr;
    }
    return this->grid[(int)position.x][(int)position.y];
}

void World::clear() {
    for (Bot* bot : bots) { delete bot; }
    bots.clear();
    for (int x = 0; x < grid.size(); x++) {
        for (int y = 0; y < grid[x].size(); y++) {
            grid[x][y] = nullptr;
        }
    }
    step_count = 0;
}

void World::saveWorld(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) return;

    out.write(reinterpret_cast<char*>(&seed), sizeof(seed));
    out.write(reinterpret_cast<char*>(&step_count), sizeof(step_count));
    size_t bot_count = bots.size();
    out.write(reinterpret_cast<char*>(&bot_count), sizeof(bot_count));

    for (Bot* bot : bots) {
        bot->serialize(out);
    }
    out.close();
}

void World::loadWorld(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) return;

    clear();

    in.read(reinterpret_cast<char*>(&seed), sizeof(seed));
    SetRandomSeed(seed);

    in.read(reinterpret_cast<char*>(&step_count), sizeof(step_count));
    size_t bot_count;
    in.read(reinterpret_cast<char*>(&bot_count), sizeof(bot_count));

    for (size_t i = 0; i < bot_count; ++i) {
        Bot* bot = new Bot();
        bot->deserialize(in);
        addBot(bot);
    }
    in.close();
}
