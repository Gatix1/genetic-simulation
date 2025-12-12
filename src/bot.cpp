#include <bot.h>
#include <world.h>
#include <algorithm>
#include <stdexcept>


Bot::Bot() {
    this->color = {
        (unsigned char)GetRandomValue(50, 200),
        (unsigned char)GetRandomValue(50, 200),
        (unsigned char)GetRandomValue(50, 200),
        255
    };
    _initRandomGenome();
}

Vector2 Bot::getPosition() {
    return this->position;
}

int Bot::getEnergy() const { return this->energy; }
Color Bot::getColor() const { return this->color; }

void Bot::addEnergy(int amount) {
    this->energy = std::min(MAX_ENERGY, this->energy + amount);
}

void Bot::_initRandomGenome() {
    for(int i = 0; i < GENOME_SIZE; i++) {
        this->genome[i] = GetRandomValue(0, 127); // Instructions are 0..127 (128 total)
    }
}

void Bot::_constrainPosition() {
    if (this->position.x < 0)
        this->position.x = WORLD_WIDTH - 1;
    if (this->position.x >= WORLD_WIDTH)
        this->position.x = 0;
    if (this->position.y < 0)
        this->position.y = WORLD_HEIGHT - 1;
    if (this->position.y >= WORLD_HEIGHT)
        this->position.y = 0;
}

void Bot::_move(int relative_index, World& world) {
    // relative_index: number from 0 to 7, 0 being top-left, 7 being left, clockwise
    if ((relative_index < 0) || (relative_index > 7)) {
        return;
    }
    int RELATIVE_INDEX_TO_OFFSET[] = {
        0,  // 0: Forward (0 degrees)
        1,  // 1: DiagRight (+45 degrees)
        2,  // 2: Right (+90 degrees)
        3,  // 3: FarRight (+135 degrees)
        4,  // 4: Back (180 degrees)
        -3, // 5: FarLeft (-135 degrees)
        -2, // 6: Left (-90 degrees)
        -1  // 7: DiagLeft (-45 degrees)
    };
    Vector2 DIRECTIONS[] = {
        {-1, -1}, // 0: NORTHWEST (Top-Left)
        { 0, -1}, // 1: NORTH
        { 1, -1}, // 2: NORTHEAST
        { 1,  0}, // 3: EAST
        { 1,  1}, // 4: SOUTHEAST
        { 0,  1}, // 5: SOUTH
        {-1,  1}, // 6: SOUTHWEST
        {-1,  0}  // 7: WEST
    };

    // Look up the steps needed for the requested relative index
    int offset = RELATIVE_INDEX_TO_OFFSET[int(relative_index)];
    
    // Calculate the target direction index using modular arithmetic
    unsigned int target_direction_index = (this->direction + offset) % 8;
    
    // Get the movement vector (dx, dy)
    Vector2 dpos = DIRECTIONS[int(target_direction_index)];
    
    Vector2 target_pos = { this->position.x + dpos.x, this->position.y + dpos.y };

    // Check for horizontal boundaries
    if (target_pos.x < 0 || target_pos.x >= WORLD_WIDTH) {
        return; // Hit a solid horizontal wall, do not move.
    }

    // Wrap vertically
    if (target_pos.y < 0) target_pos.y = WORLD_HEIGHT - 1;
    if (target_pos.y >= WORLD_HEIGHT) target_pos.y = 0;

    if (world.getBotAt(target_pos) != nullptr) return; // Target cell is occupied, do not move.

    Vector2 old_pos = this->position;

    // Update position to the calculated target position
    this->position = target_pos;

    this->_constrainPosition(); // Constrain before updating grid

    // Notify the world about the position change to keep the grid synchronized.
    world.updateBotPosition(this, old_pos);
    this->energy -= 1;
}

/*
* Overload of _constrainPosition to apply constraints to a passed Vector2
*/
void Bot::_constrainPosition(Vector2 &pos) {
    if (pos.x < 0)
        pos.x = 0; // Clamp to left edge
    if (pos.x >= WORLD_WIDTH)
        pos.x = WORLD_WIDTH - 1; // Clamp to right edge
    if (pos.y < 0)
        pos.y = WORLD_HEIGHT - 1;
    if (pos.y >= WORLD_HEIGHT)
        pos.y = 0;
}

void Bot::_turn(int relative_index) {
    // relative_index: number from 0 to 7, 0 being top-left, 7 being left, clockwise
    if ((relative_index < 0) || (relative_index > 7)) {
        return;
    }

    this->direction += relative_index;
    this->direction %= 8;
}

void Bot::_look(int relative_index, World& world) {
    // relative_index: number from 0 to 7, 0 being top-left, 7 being left, clockwise
    if ((relative_index < 0) || (relative_index > 7)) {
        return;
    }
    int RELATIVE_INDEX_TO_OFFSET[] = {
        0,  // 0: Forward (0 degrees)
        1,  // 1: DiagRight (+45 degrees)
        2,  // 2: Right (+90 degrees)
        3,  // 3: FarRight (+135 degrees)
        4,  // 4: Back (180 degrees)
        -3, // 5: FarLeft (-135 degrees)
        -2, // 6: Left (-90 degrees)
        -1  // 7: DiagLeft (-45 degrees)
    };
    Vector2 DIRECTIONS[] = {
        {-1, -1}, // 0: NORTHWEST (Top-Left)
        { 0, -1}, // 1: NORTH
        { 1, -1}, // 2: NORTHEAST
        { 1,  0}, // 3: EAST
        { 1,  1}, // 4: SOUTHEAST
        { 0,  1}, // 5: SOUTH
        {-1,  1}, // 6: SOUTHWEST
        {-1,  0}  // 7: WEST
    };
    // Look up the steps needed for the requested relative index
    int offset = RELATIVE_INDEX_TO_OFFSET[int(relative_index)];
    
    // Calculate the target direction index using modular arithmetic
    unsigned int target_direction_index = (this->direction + offset) % 8;
    
    // Get the looking vector (dx, dy)
    Vector2 dpos = DIRECTIONS[int(target_direction_index)];

    Vector2 target_pos = { this->position.x + dpos.x, this->position.y + dpos.y };

    Bot* target_bot_ptr = world.getBotAt(target_pos);
    if (target_bot_ptr != nullptr) {
        if (target_bot_ptr->isOrganic) {
            this->pc += 3; // It's organic matter
        } else {
            this->pc += 2; // It's another living bot
        }
    } else {
        this->pc += 1; // It's empty
    }
}

void Bot::_attack(int relative_index, World& world) {
    // relative_index: number from 0 to 7, 0 being top-left, 7 being left, clockwise
    if ((relative_index < 0) || (relative_index > 7)) {
        return;
    }
    int RELATIVE_INDEX_TO_OFFSET[] = {
        0,  // 0: Forward (0 degrees)
        1,  // 1: DiagRight (+45 degrees)
        2,  // 2: Right (+90 degrees)
        3,  // 3: FarRight (+135 degrees)
        4,  // 4: Back (180 degrees)
        -3, // 5: FarLeft (-135 degrees)
        -2, // 6: Left (-90 degrees)
        -1  // 7: DiagLeft (-45 degrees)
    };
    Vector2 DIRECTIONS[] = {
        {-1, -1}, // 0: NORTHWEST (Top-Left)
        { 0, -1}, // 1: NORTH
        { 1, -1}, // 2: NORTHEAST
        { 1,  0}, // 3: EAST
        { 1,  1}, // 4: SOUTHEAST
        { 0,  1}, // 5: SOUTH
        {-1,  1}, // 6: SOUTHWEST
        {-1,  0}  // 7: WEST
    };

    this->energy -= 10;

    // Look up the steps needed for the requested relative index
    int offset = RELATIVE_INDEX_TO_OFFSET[int(relative_index)];
    
    // Calculate the target direction index using modular arithmetic
    unsigned int target_direction_index = (this->direction + offset) % 8;
    
    // Get the attack vector (dx, dy)
    Vector2 dpos = DIRECTIONS[int(target_direction_index)];

    Vector2 target_pos = {this->position.x + dpos.x, this->position.y + dpos.y};
    _constrainPosition(target_pos);

    Bot *target_bot_ptr = world.getBotAt(target_pos);
    if (target_bot_ptr != nullptr && target_bot_ptr != this && !target_bot_ptr->isOrganic) {
        this->nutrition_balance = std::max(-20, this->nutrition_balance - 10); // Become more carnivorous
        this->scavenge_points = std::max(0, this->scavenge_points - 2); // Attacking is not scavenging
        target_bot_ptr->die(world); // The attacked bot becomes organic matter
    }
}

int Bot::_genomeDifference(const Bot& other) const {
    int differences = 0;
    for (size_t i = 0; i < this->genome.size(); ++i) {
        if (this->genome[i] != other.genome[i]) {
            differences++;
        }
    }
    return differences;
}

void Bot::_checkRelative(int relative_index, World& world) {
    if ((relative_index < 0) || (relative_index > 7)) {
        return;
    }
    int RELATIVE_INDEX_TO_OFFSET[] = { 0, 1, 2, 3, 4, -3, -2, -1 };
    Vector2 DIRECTIONS[] = {
        {-1, -1}, { 0, -1}, { 1, -1}, { 1,  0}, { 1,  1}, { 0,  1}, {-1,  1}, {-1,  0}
    };

    int offset = RELATIVE_INDEX_TO_OFFSET[relative_index];
    unsigned int target_direction_index = (this->direction + offset + 8) % 8;
    Vector2 dpos = DIRECTIONS[target_direction_index];
    Vector2 target_pos = { this->position.x + dpos.x, this->position.y + dpos.y };

    Bot* target_bot = world.getBotAt(target_pos);
    if (target_bot != nullptr && target_bot != this) {
        if (_genomeDifference(*target_bot) < 2) {
            this->pc += 2; // If 0..1 difference - It's a relative, skip next instruction
            return;
        }
    }
    this->pc += 1; // Not a relative or no bot found
}

void Bot::_shareEnergy(int relative_index, World& world) {
    if ((relative_index < 0) || (relative_index > 7)) {
        return;
    }
    int RELATIVE_INDEX_TO_OFFSET[] = { 0, 1, 2, 3, 4, -3, -2, -1 };
    Vector2 DIRECTIONS[] = {
        {-1, -1}, { 0, -1}, { 1, -1}, { 1,  0}, { 1,  1}, { 0,  1}, {-1,  1}, {-1,  0}
    };

    int energy_to_share = this->energy * 0.1;
    if (energy_to_share <= 0) {
        return; // Nothing to share
    }

    int offset = RELATIVE_INDEX_TO_OFFSET[relative_index];
    unsigned int target_direction_index = (this->direction + offset + 8) % 8;
    Vector2 dpos = DIRECTIONS[target_direction_index];
    Vector2 target_pos = { this->position.x + dpos.x, this->position.y + dpos.y };

    Bot* target_bot = world.getBotAt(target_pos);
    if (target_bot != nullptr && target_bot != this) {
        this->energy -= energy_to_share;
        target_bot->addEnergy(energy_to_share);
    }
}

void Bot::_consumeOrganic(int relative_index, World& world) {
    // relative_index: number from 0 to 7
    if ((relative_index < 0) || (relative_index > 7)) {
        return;
    }
    int RELATIVE_INDEX_TO_OFFSET[] = {
        0, 1, 2, 3, 4, -3, -2, -1
    };
    Vector2 DIRECTIONS[] = {
        {-1, -1}, { 0, -1}, { 1, -1}, { 1,  0}, { 1,  1}, { 0,  1}, {-1,  1}, {-1,  0}
    };

    // Look up the steps needed for the requested relative index
    int offset = RELATIVE_INDEX_TO_OFFSET[relative_index];
    
    // Calculate the target direction index using modular arithmetic
    unsigned int target_direction_index = (this->direction + offset + 8) % 8;
    
    // Get the target vector (dx, dy)
    Vector2 dpos = DIRECTIONS[target_direction_index];

    Vector2 target_pos = { this->position.x + dpos.x, this->position.y + dpos.y };
    _constrainPosition(target_pos);

    Bot *target_bot_ptr = world.getBotAt(target_pos);
    if (target_bot_ptr != nullptr && target_bot_ptr->isOrganic) {
        this->addEnergy(target_bot_ptr->energy);
        this->scavenge_points = std::min(20, this->scavenge_points + 10); // Mark as a scavenger
        world.removeBot(target_bot_ptr); // The organic matter is consumed and disappears
    }
    // No energy cost for consuming
    this->pc++;
}

Vector2 Bot::_findEmptyAdjacentCell(World &world) {
    std::vector<Vector2> directions = {
        {-1, -1}, { 0, -1}, { 1, -1}, // NW, N, NE
        {-1,  0},           { 1,  0}, // W, E
        {-1,  1}, { 0,  1}, { 1,  1}  // SW, S, SE
    };

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(directions.begin(), directions.end(), g);

    for (const auto& dir : directions) {
        Vector2 target_pos = {this->position.x + dir.x, this->position.y + dir.y};

        // Do not wrap horizontally, just check if it's a valid position
        if (target_pos.x < 0 || target_pos.x >= WORLD_WIDTH) continue;

        // Wrap vertically
        if (target_pos.y < 0) target_pos.y = WORLD_HEIGHT - 1;
        if (target_pos.y >= WORLD_HEIGHT) target_pos.y = 0;

        if (world.getBotAt(target_pos) == nullptr) {
            return target_pos; // Found an empty cell
        }
    }

    return {-1, -1}; // Return an invalid position if no empty adjacent cell is found
}

void Bot::_reproduce(World& world) {
    // A bot needs a certain amount of energy to reproduce.
    if (this->energy < REPRODUCTION_ENERGY_MINIMUM) {
        return;
    }

    Vector2 spawnPosition = this->_findEmptyAdjacentCell(world);
    if (spawnPosition.x == -1 && spawnPosition.y == -1) {
        return;
    }

    int childEnergy = this->energy / 2;
    this->energy = childEnergy;

    Bot* child = new Bot();
    child->position = spawnPosition;
    child->energy = childEnergy;
    child->color = this->color;
    child->nutrition_balance = 0; // Child starts with a neutral dietary balance
    child->genome = this->genome; // This performs a deep copy of the parent's genome
    for (int i = 0; i < child->genome.size(); i++) {
        // Check for genome mutation.
        if (GetRandomValue(1, 10000) <= (int)(MUTATION_RATE * 10000.0f)) {
            child->genome[i] = GetRandomValue(0, 127);

            // If the genome mutates, also mutate the color slightly.
            child->color.r = std::clamp(child->color.r + GetRandomValue(-COLOR_MUTATION_AMOUNT, COLOR_MUTATION_AMOUNT), 0, 255);
            child->color.g = std::clamp(child->color.g + GetRandomValue(-COLOR_MUTATION_AMOUNT, COLOR_MUTATION_AMOUNT), 0, 255);
            child->color.b = std::clamp(child->color.b + GetRandomValue(-COLOR_MUTATION_AMOUNT, COLOR_MUTATION_AMOUNT), 0, 255);
        }
    }

    world.addBot(child);
}

void Bot::setPosition(Vector2 pos) {
    this->position = pos;
}

void Bot::render(int view_mode) {
    this->render(view_mode, 255); // Call the main render function with full opacity
}

void Bot::render(int view_mode, unsigned char alpha_override) {
    Color render_color = this->color;

    if (this->isOrganic) {
        float margin = CELL_SIZE / 4.0f;
        DrawRectangle(this->position.x * CELL_SIZE + margin, this->position.y * CELL_SIZE + margin, CELL_SIZE - margin * 2, CELL_SIZE - margin * 2, {DARKGRAY.r, DARKGRAY.g, DARKGRAY.b, alpha_override});
        return;
    }

    switch (view_mode) {
        case 1: { // Nutrition
            // Use nutrition_balance for herbivore/carnivore and scavenge_points for scavenger
            if (this->nutrition_balance > 0) { // Herbivore (Green)
                float ratio = std::clamp(this->nutrition_balance / 20.0f, 0.0f, 1.0f);
                render_color = { (unsigned char)(255 * (1.0f - ratio)), 255, 0, 255 }; // Yellow to Green
            } else { // Carnivore or Scavenger
                if (this->scavenge_points > -this->nutrition_balance) { // Primarily a Scavenger (Blue)
                    float ratio = std::clamp(this->scavenge_points / 20.0f, 0.0f, 1.0f);
                    render_color = { (unsigned char)(255 * (1.0f - ratio)), (unsigned char)(255 * (1.0f - ratio)), 255, 255 }; // Yellow to Blue
                } else { // Primarily a Predator (Red)
                    float ratio = std::clamp(-this->nutrition_balance / 20.0f, 0.0f, 1.0f);
                    render_color = { 255, (unsigned char)(255 * (1.0f - ratio)), 0, 255 }; // Yellow to Red
                }
            }
            break;
        }
        case 2: { // Energy Level
            float energy_ratio = std::clamp((float)this->energy / (float)INITIAL_ENERGY, 0.0f, 1.0f);
            // Simple gradient from Red to Green
            render_color.r = 255;
            render_color.g = (unsigned char)(255 * energy_ratio); // Green component increases with energy
            render_color.b = 0;
            break;
        }
        case 3: // Species Color (default)
            // render_color is already this->color
            break;
    }

    render_color.a = alpha_override;
    DrawRectangle(this->position.x * CELL_SIZE, this->position.y * CELL_SIZE, CELL_SIZE, CELL_SIZE, render_color);
}

void Bot::_processGenome(World &world) {
    this->pc %= this->genome.size();
    unsigned int instruction = this->genome[this->pc];

    // 0..7 Move Relative
    if (instruction < 8) {
        this->_move(instruction, world);
        this->pc++;
    }
    // 8..15 Turn Relatively
    else if (instruction < 16) {
        this->_turn(instruction - 8);
        this->pc++;
    }
    // 16..23 Look Relatively
    else if (instruction < 24) {
        this->_look(instruction - 16, world);
        // pc is incremented inside _look
    }
    // 24..31 Attack relatively
    else if (instruction < 32) {
        this->_attack(instruction - 24, world);
        this->pc++;
    }
    // 32..39 Photosynthize (free energy) 8 instructions to make this behavior more common
    else if (instruction < 40) {
        int energy_gain = HIGH_PHOTOSYNTHIZE_ENERGY_GAIN; // Balanced biome (center)
        float bot_x = this->position.x;

        if (bot_x < WORLD_WIDTH / 3.0f) {
            energy_gain = PHOTOSYNTHIZE_ENERGY_GAIN; // Sunny biome (left)
        } else if (bot_x >= 2.0f * WORLD_WIDTH / 3.0f) {
            energy_gain = LOW_PHOTOSYNTHIZE_ENERGY_GAIN; // Dark biome (right)
        }

        this->energy = std::min(MAX_ENERGY, this->energy + energy_gain);
        this->nutrition_balance = std::min(20, this->nutrition_balance + 1); // Become more vegetarian
        this->scavenge_points = std::max(0, this->scavenge_points - 1); // Photosynthesis is not scavenging
        this->pc++;
    }
    // 40..47 Check if neighbor is a relative
    else if (instruction < 48) {
        this->_checkRelative(instruction - 40, world);
        // pc is incremented inside _checkRelative
    }
    // 48..55 Share energy with neighbor
    else if (instruction < 56) {
        this->_shareEnergy(instruction - 48, world);
        this->pc++;
    }
    // 56..60 Consume Organic
    else if (instruction < 61) {
        this->_consumeOrganic(instruction - 56, world);
        // pc is incremented inside _consumeOrganic
    }
    // 61 Reproduce
    else if (instruction == 61) {
        this->_reproduce(world);
        this->pc++;
    }
    // 62 (Create adjusting bot) - Replaced to unconditional jump for now.
    else if (instruction == 62) {
        // Here was some other functionality, i replaced it with an uncodintional jump for now.
        this->pc += this->genome[(this->pc + 1) % GENOME_SIZE] % 10;
    }
    // 63 Check biome
    else if (instruction == 63) {
        this->_checkBiome();
    }
    // 64..127 Unconditional Jump (Default action)
    else {
        this->pc += this->genome[(this->pc + 1) % GENOME_SIZE] % 10; // Jump 0-9 forward
    }
}

void Bot::_checkBiome() {
    if (this->position.x < WORLD_WIDTH / 3.0f) this->pc += 1; // Sunny biome
    else if (this->position.x < 2.0f * WORLD_WIDTH / 3.0f) this->pc += 2; // Balanced biome
    else this->pc += 3; // Dark biome
}

void Bot::process(World& world) {
    this->age++;

    if (this->isOrganic) {
        // Organic matter "falls" to the right if there's space.
        Vector2 target_pos = { this->position.x + 1, this->position.y };

        // Check if the target is within horizontal bounds and is empty.
        if (target_pos.x < WORLD_WIDTH && world.getBotAt(target_pos) == nullptr) {
            Vector2 old_pos = this->position;
            this->position = target_pos;
            world.updateBotPosition(this, old_pos);
        }
        return; // Organic matter does nothing else.
    }

    this->energy -= 1;

    // Check for death conditions
    if (this->energy <= 0) {
        // Death by starvation: bot disappears without creating organic matter.
        world.removeBot(this);
        return;
    }
    if (this->age > MAXIMUM_BOT_AGE) {
        // Death by old age: bot becomes organic matter with its remaining energy.
        this->die(world);
        return;
    }

    this->_processGenome(world);
}

void Bot::die(World& world) {
    this->isOrganic = true;
    // The corpse retains the energy the bot had at the moment of death.
}

void Bot::serialize(std::ofstream& out) {
    out.write(reinterpret_cast<char*>(&position), sizeof(position));
    out.write(reinterpret_cast<char*>(&energy), sizeof(energy));
    out.write(reinterpret_cast<char*>(&age), sizeof(age));
    
    size_t genome_size = genome.size();
    out.write(reinterpret_cast<char*>(&genome_size), sizeof(genome_size));
    out.write(reinterpret_cast<char*>(genome.data()), genome_size * sizeof(unsigned int));

    out.write(reinterpret_cast<char*>(&pc), sizeof(pc));
    out.write(reinterpret_cast<char*>(&color), sizeof(color));
    out.write(reinterpret_cast<char*>(&direction), sizeof(direction));
    out.write(reinterpret_cast<char*>(&is_dead), sizeof(is_dead));
    out.write(reinterpret_cast<char*>(&isOrganic), sizeof(isOrganic));
    out.write(reinterpret_cast<char*>(&nutrition_balance), sizeof(nutrition_balance));
    out.write(reinterpret_cast<char*>(&scavenge_points), sizeof(scavenge_points));
}

void Bot::deserialize(std::ifstream& in) {
    in.read(reinterpret_cast<char*>(&position), sizeof(position));
    in.read(reinterpret_cast<char*>(&energy), sizeof(energy));
    in.read(reinterpret_cast<char*>(&age), sizeof(age));

    size_t genome_size;
    in.read(reinterpret_cast<char*>(&genome_size), sizeof(genome_size));
    genome.resize(genome_size);
    in.read(reinterpret_cast<char*>(genome.data()), genome_size * sizeof(unsigned int));

    in.read(reinterpret_cast<char*>(&pc), sizeof(pc));
    in.read(reinterpret_cast<char*>(&color), sizeof(color));
    in.read(reinterpret_cast<char*>(&direction), sizeof(direction));
    in.read(reinterpret_cast<char*>(&is_dead), sizeof(is_dead));
    in.read(reinterpret_cast<char*>(&isOrganic), sizeof(isOrganic));
    in.read(reinterpret_cast<char*>(&nutrition_balance), sizeof(nutrition_balance));
    in.read(reinterpret_cast<char*>(&scavenge_points), sizeof(scavenge_points));
}