#include <bot.h>
#include <string>
#pragma once

class World {
public:
    World();
    ~World();
    void newWorld(unsigned int seed, int initial_bot_count);
    void spawnInitialBots(int count);
    void addBot(Bot *bot_ptr);
    void removeBot(Bot* bot_ptr);
    void render(int view_mode, Bot* organism_root);
    void process();
    void updateBotPosition(Bot* bot_ptr, Vector2 old_pos);
    Bot* getBotAt(Vector2 position);
    int getBotsSize() const { return this->bots.size(); }
    long long getStepCount() const { return this->step_count; }
    unsigned int getSeed() const { return this->seed; }
    void saveWorld(const std::string& filename);
    void loadWorld(const std::string& filename);
    void clear();
private:
    std::vector<Bot*> bots;
    Bot* grid[WORLD_WIDTH][WORLD_HEIGHT] = {0};
    long long step_count = 0;
    unsigned int seed = 0;
};