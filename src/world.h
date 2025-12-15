#include <bot.h>
#include <string>
#pragma once

class World {
public:
    World(int width, int height);
    World();
    ~World();
    void newWorld(unsigned int seed, int initial_bot_count);
    void spawnInitialBots(int count);
    void addBot(Bot *bot_ptr);
    void removeBot(Bot* bot_ptr);
    void render(int view_mode, Bot* selected_bot, const std::vector<Bot*>& relatives);
    void process();
    void updateBotPosition(Bot* bot_ptr, Vector2 old_pos);
    Bot* getBotAt(Vector2 position);
    const std::vector<Bot*>& getBots() const;
    int getBotsSize() const { return this->bots.size(); }
    long long getStepCount() const { return this->step_count; }
    unsigned int getSeed() const { return this->seed; }
    void saveWorld(const std::string& filename);
    void loadWorld(const std::string& filename); 
    void clear();
    int getWidth() const { return world_width; }
    int getHeight() const { return world_height; }
private:
    std::vector<Bot*> bots;
    std::vector<std::vector<Bot*>> grid;
    int world_width;
    int world_height;
    long long step_count = 0;
    unsigned int seed = 0;
};