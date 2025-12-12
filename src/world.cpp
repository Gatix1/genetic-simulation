#include <world.h>
#include <algorithm>
#include <stdexcept>
#include <fstream>

World::World() {}

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
        const int max_attempts = WORLD_WIDTH * WORLD_HEIGHT;

        // Find a random empty cell for the new bot
        do {
            spawn_pos = {(float)GetRandomValue(0, WORLD_WIDTH - 1), (float)GetRandomValue(0, WORLD_HEIGHT - 1)};
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
    this->grid[(int)old_pos.x][(int)old_pos.y] = nullptr;
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
    // Sunny Biome (Left)
    DrawRectangle(0, 0, (WORLD_WIDTH / 3) * CELL_SIZE, WORLD_HEIGHT * CELL_SIZE, {255, 200, 0, 40});
    // Normal Biome (Center)
    DrawRectangle((WORLD_WIDTH / 3) * CELL_SIZE, 0, (WORLD_WIDTH / 3) * CELL_SIZE, WORLD_HEIGHT * CELL_SIZE, {0, 255, 100, 40});
    // Dark Biome (Right)
    DrawRectangle((2 * WORLD_WIDTH / 3) * CELL_SIZE, 0, (WORLD_WIDTH / 3) * CELL_SIZE, WORLD_HEIGHT * CELL_SIZE, {0, 255, 255, 40});

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
    for (int i = 0; i < WORLD_WIDTH; i++) {
        DrawLineEx({float(i) * CELL_SIZE, 0},
                   {float(i) * CELL_SIZE, WORLD_HEIGHT * CELL_SIZE},
                   GRID_THICKNESS, GRID_COLOR);
    }
    for (int i = 0; i < WORLD_HEIGHT; i++) {
        DrawLineEx({0, float(i) * CELL_SIZE},
                   {WORLD_WIDTH * CELL_SIZE, float(i) * CELL_SIZE},
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
            delete bot; // Now it's safe to free the memory.
            return true;
        }
        return false;
    });
    this->bots.erase(it, this->bots.end());
}

Bot* World::getBotAt(Vector2 position) {
    if (position.x < 0 || position.x >= WORLD_WIDTH || position.y < 0 || position.y >= WORLD_HEIGHT) {
        return nullptr;
    }
    return this->grid[(int)position.x][(int)position.y];
}

void World::clear() {
    for (Bot* bot : bots) {
        delete bot;
    }
    bots.clear();
    for (int x = 0; x < WORLD_WIDTH; x++) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
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
