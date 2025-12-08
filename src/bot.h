#include <raylib.h>
#include <vector>
#include <random>
#include <config.h>
#pragma once

class World; // Forward declaration

class Bot {
public:
    Bot();
    void render(int view_mode);
    void render(int view_mode, unsigned char alpha_override);
    void process(World& world);
    Vector2 getPosition();
    int getEnergy() const;
    Color getColor() const;
    void addEnergy(int amount);
    void setPosition(Vector2 pos);
    bool is_dead = false;
    bool isOrganic = false;
private:
    Vector2 position;
    int energy = INITIAL_ENERGY;
    int age = 0;
    std::vector<unsigned int> genome = std::vector<unsigned int>(GENOME_SIZE);
    unsigned int pc = 0; // program counter, the index of current action in genome
    Color color = {0, 0, 255, 255}; // Default color is blue
    unsigned int direction = 1; // 0..7
    void _constrainPosition();
    void die(World& world);
    void _reproduce(World& world);
    Vector2 _findEmptyAdjacentCell(World& world);
    void _processGenome(World& world);
    void _attack(int relative_index, World& world);
    void _look(int relative_index, World& world);
    void _turn(int relative_index);
    void _move(int relative_index, World& world);
    void _initRandomGenome();
    void _checkRelative(int relative_index, World& world);
    void _shareEnergy(int relative_index, World& world);
    void _checkBiome();
    void _consumeOrganic(int relative_index, World& world);
    int _genomeDifference(const Bot& other) const;
    void _constrainPosition(Vector2 &pos);
    int nutrition_balance = 0; // Negative for carnivore, positive for vegetarian
    int scavenge_points = 0; // Tracks how much a bot has scavenged (modified by eating corpses)
};