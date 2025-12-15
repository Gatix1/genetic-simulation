#include "GenomeAnalyzer.h"
#include "rlImGui.h"
#include "imgui.h"
#include "instructions.h"
#include <string>
#include <map>
#include <functional>

// A map to associate instruction enum values with human-readable string names.
const std::map<int, std::string> instruction_names = {
    {MOVE, "MOVE"}, {TURN, "TURN"}, {LOOK, "LOOK"}, {ATTACK, "ATTACK"},
    {PHOTOSYNTHIZE, "PHOTO"}, {CHECK_RELATIVE, "CH_RELATIVE"},
    {SHARE_ENERGY, "SHARE"}, {CONSUME_ORGANIC, "EAT_ORGANIC"},
    {REPRODUCE, "REPRODUCE"},
    {CHECK_BIOME, "CH_BIOME"}, {CHECK_X, "CHECK_X"}, {CHECK_Y, "CHECK_Y"},
    {CHECK_ENERGY, "CHECK_ENERGY"}, {CHECK_AGE, "CHECK_AGE"},
    {JUMP_IF_EQUAL, "JMP_EQ"}, {JUMP_IF_NOT_EQUAL, "JMP_NE"}, {JUMP_IF_GREATER, "JMP_GT"}
};

/**
 * @brief Converts an instruction value to its string representation.
 * @param instruction The unsigned integer value of the instruction.
 * @return A string name for the instruction.
 */
std::string getInstructionName(unsigned int instruction) {
    if (instruction_names.count(instruction)) {
        return instruction_names.at(instruction);
    }
    if (instruction >= JUMP && instruction <= MAX_INSTRUCTION_VALUE) {
        return "JMP";
    }
    return "UNKNOWN";
}

GenomeAnalyzer::GenomeAnalyzer() {}

// Destructor ensures the local simulation world is cleaned up.
GenomeAnalyzer::~GenomeAnalyzer() {
    delete local_world;
}

// Returns true if the analyzer window is currently open.
bool GenomeAnalyzer::isOpen() const {
    return is_open;
}

// Closes the analyzer, cleaning up all simulation-specific data.
void GenomeAnalyzer::close() {
    is_open = false;
    current_placement_mode = PLACE_NONE;
    sim_bot = nullptr;
    delete local_world; // This will delete the sim_bot and other bots inside it
    local_world = nullptr;
    original_bot = nullptr;
}

// Opens the analyzer for a given bot, pausing the main simulation.
void GenomeAnalyzer::analyze(Bot* bot) {
    if (!bot) return;

    original_bot = bot;
    is_open = true;
    is_paused = true;

    resetSimulation();
}

// Resets the local simulation to its initial state based on the original bot.
void GenomeAnalyzer::resetSimulation() {
    delete local_world;

    // Create a deep copy of the bot for local simulation and place it in the center
    sim_bot = new Bot(*original_bot);
    sim_bot->setPosition({(float)(LOCAL_WORLD_SIZE / 2), (float)(LOCAL_WORLD_SIZE / 2)});

    // Create a small local world for the simulation
    local_world = new World(LOCAL_WORLD_SIZE, LOCAL_WORLD_SIZE);
    local_world->addBot(sim_bot);

    buildGraphLayout();
}

// Advances the local simulation by one step, processing the bot's genome.
void GenomeAnalyzer::step() {
    if (sim_bot && local_world) {
        local_world->process();
        // If the main bot died during the process() call, it has been deallocated.
        // We must not access it further. The is_dead check handles this.
        if (sim_bot == nullptr) {
            return;
        }
        if (sim_bot->is_dead) {
            is_paused = true; // Auto-pause on death of the main bot
        }
    }
}

void GenomeAnalyzer::draw() {
    // Don't draw if the window is not open.
    if (!is_open || !original_bot) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Genome Analyzer", &is_open)) {
        // Handle keyboard shortcuts for play/pause and step when the window is focused.
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            // RMB cancels placement mode
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                current_placement_mode = PLACE_NONE;
            }

            if (ImGui::IsKeyPressed(ImGuiKey_Space)) is_paused = !is_paused;
            if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) { is_paused = true; step(); }
            if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                if (current_placement_mode != PLACE_NONE) {
                    current_placement_mode = PLACE_NONE;
                } else {
                    is_open = false;
                }
            }
        }

        // If not paused, automatically advance the simulation.
        if (!is_paused) {
            step();
        }

        drawControls();
        ImGui::Separator();

        // --- Left Pane: Genome Graph ---
        ImGui::BeginChild("LeftPane", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, 0), false);
        ImGui::Text("Genome Program Flow Graph");
        // The graph is in its own child window to enable scrolling for large genomes.
        ImGui::BeginChild("Graph", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
            drawGenomeGraph();
        ImGui::EndChild();
        ImGui::EndChild();

        ImGui::SameLine();

        // --- Right Pane: Simulation Visualization and Bot State ---
        ImGui::BeginChild("RightPane", ImVec2(0, 0), false);
            // Top-right: Mini-world visualization.
            ImGui::Text("Local Simulation");
            ImGui::BeginChild("BotVisualization", ImVec2(0, ImGui::GetContentRegionAvail().y * 0.5f), true);
                bot_vis_pos = ImGui::GetCursorScreenPos();
                bot_vis_size = ImGui::GetContentRegionAvail();
                drawBotVisualization();
            ImGui::EndChild();

            drawPlacementControls();

            // Bottom-right: Bot's internal state (energy, memory, etc.).
            ImGui::Separator();
            ImGui::Text("Bot State & Memory");
            ImGui::BeginChild("BotState", ImVec2(0, 0), true);
                drawBotState();
            ImGui::EndChild();
        ImGui::EndChild();
    }
    ImGui::End();

    // If the window was closed by the user (clicking 'x'), perform cleanup.
    if (!is_open) {
        close(); // Cleanup if window was closed
    }
}

// Draws the main control buttons for the local simulation.
void GenomeAnalyzer::drawControls() {
    if (ImGui::Button(is_paused ? "Run (Space)" : "Pause (Space)")) {
        is_paused = !is_paused;
    }
    ImGui::SameLine();
    if (ImGui::Button("Step > (Right Arrow)")) {
        is_paused = true;
        step();
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        resetSimulation();
    }
}

/**
 * @brief Builds the visual layout of the genome graph.
 * This function performs a recursive traversal (DFS) starting from PC 0 to identify all reachable
 * instructions. It then positions them in a hierarchical layout to make control flow clear.
 */
void GenomeAnalyzer::buildGraphLayout() {
    node_positions.clear();
    if (!sim_bot) return;

    const auto& genome = sim_bot->getGenome();
    if (genome.empty()) return;

    node_positions.assign(genome.size(), ImVec2(-1, -1)); // Use -1,-1 to mark non-reachable/not-yet-placed

    // Constants for controlling the graph's appearance.
    const float LEVEL_HEIGHT = 50.0f + 50.0f;
    const float NODE_SPACING_X = 170.0f + 40.0f;

    std::vector<int> nodes_at_depth(genome.size(), 0);

    // A recursive lambda function to place nodes.
    std::function<void(int, int, int)> place_node_recursive;
    place_node_recursive = 
        [&](int pc, int depth, int parent_col) {

            // Base cases for recursion: out of bounds or already visited.
            if (pc < 0 || pc >= genome.size()) return;
            // If node is already placed at this or a shallower depth, stop to prevent cycles/clutter.
            // This makes backward jumps (loops) use the "GOTO" rendering instead of drawing long lines up.
            if (node_positions[pc].x != -1 && node_positions[pc].y <= (float)(depth * LEVEL_HEIGHT + 50)) {
                return;
            }
            // Calculate the position for the current node.
            int current_col = std::max(parent_col, nodes_at_depth[depth]);
            node_positions[pc] = ImVec2((float)(current_col * NODE_SPACING_X + 50), (float)(depth * LEVEL_HEIGHT + 50));
            // Update layout metadata to prevent future nodes from overlapping.
            nodes_at_depth[depth] = current_col + 1;
            for (int i = depth + 1; i < genome.size(); ++i) {
                nodes_at_depth[i] = std::max(nodes_at_depth[i], current_col);
            }

            unsigned int instruction = genome[pc];
            
            // Recursively call for the next nodes based on the current instruction type.
            if (instruction == LOOK) {
                place_node_recursive((pc + 1) % genome.size(), depth + 1, current_col); // Empty
                place_node_recursive((pc + 2) % genome.size(), depth + 1, current_col); // Bot
                place_node_recursive((pc + 3) % genome.size(), depth + 1, current_col); // Organic
            } else if (instruction >= JUMP_IF_EQUAL && instruction <= JUMP_IF_GREATER) {
                int jump_offset = (pc + 1 < genome.size()) ? (genome[pc + 1] % 10) : 0;
                place_node_recursive((pc + jump_offset) % genome.size(), depth + 1, current_col); // True path (down)
                place_node_recursive((pc + 2) % genome.size(), depth + 1, current_col + 1); // False path (right)
            } else if (instruction >= JUMP && instruction <= MAX_INSTRUCTION_VALUE) {
                // Unconditional jump
                place_node_recursive((pc + instruction) % genome.size(), depth + 1, current_col);
            } else {
                place_node_recursive((pc + 1) % genome.size(), depth + 1, current_col);
            }
        };

    // Start the recursive layout process from the first instruction (PC=0).
    place_node_recursive(0, 0, 0);
}

/**
 * @brief Draws the genome graph using ImGui's custom drawing API.
 * It iterates through all reachable nodes, drawing them and the connections (edges) between them.
 */
void GenomeAnalyzer::drawGenomeGraph() {
    if (node_positions.empty() || !sim_bot) return;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();

    const auto& genome = sim_bot->getGenome();
    unsigned int current_pc = sim_bot->getPC();

    float max_x = 0.0f, max_y = 0.0f;

    // --- Pass 1: Draw Edges ---
    // Edges are drawn first so they appear underneath the nodes.
    for (int i = 0; i < genome.size(); ++i) {
        if (node_positions[i].x < 0) continue; // Skip non-reachable nodes

        unsigned int instruction = genome[i];
        ImVec2 default_start_pos = ImVec2(p.x + node_positions[i].x + 75, p.y + node_positions[i].y + 15);

        // Helper lambda to draw a single edge with an optional label and arrow.
        auto draw_edge = [&](int target_idx, ImU32 color, const char* label = nullptr, bool force_bezier = false, ImVec2* start_pos_override = nullptr) {
            if (target_idx >= 0 && target_idx < genome.size() && node_positions[target_idx].x >= 0) {
                ImVec2 end_pos = ImVec2(p.x + node_positions[target_idx].x + 75, p.y + node_positions[target_idx].y + 15);

                // Use bezier for non-sequential jumps to reduce clutter
                bool is_sequential_flow = (target_idx == (i + 1) % genome.size() || target_idx == (i + 2) % genome.size() || target_idx == (i + 3) % genome.size());
                
                // Heuristic for a "long jump": a non-sequential jump that goes up more than one level, or across a large horizontal distance.
                bool is_long_jump = !is_sequential_flow && (
                    node_positions[target_idx].y < node_positions[i].y - 100.0f || // Jumps "up" (loops)
                    std::abs(node_positions[target_idx].x - node_positions[i].x) > 400.0f // Jumps far sideways
                );

                bool is_false_branch_from_conditional = (label && strcmp(label, "False") == 0);

                ImVec2 start_pos = start_pos_override ? *start_pos_override : default_start_pos;

                if (is_long_jump) { // For long jumps, draw a "GOTO N" node instead of a long line.
                    // Draw a short stub line pointing down from the node.
                    ImVec2 goto_node_pos;
                    if (is_false_branch_from_conditional) {
                        // For a 'false' branch, the GOTO should be to the right.
                        goto_node_pos = ImVec2(start_pos.x + 100.0f, start_pos.y + 60.0f);
                        draw_list->AddLine(ImVec2(start_pos.x + 20, start_pos.y + 25), goto_node_pos, color, 2.0f);
                    } else {
                        // For 'true' or unconditional jumps, the GOTO is straight down.
                        goto_node_pos = ImVec2(start_pos.x, start_pos.y + 60.0f);
                        draw_list->AddLine(start_pos, goto_node_pos, color, 2.0f);
                    }
                    
                    // Create the "GOTO" text and calculate its size.
                    std::string goto_text = "GOTO " + std::to_string(target_idx);
                    ImVec2 text_size = ImGui::CalcTextSize(goto_text.c_str());
                    ImVec2 rect_pos = ImVec2(goto_node_pos.x - text_size.x / 2.0f - 5.0f, goto_node_pos.y);
                    ImVec2 rect_end = ImVec2(rect_pos.x + text_size.x + 10, rect_pos.y + text_size.y + 4);

                    // Draw the GOTO box with a background, border, and text.
                    draw_list->AddRectFilled(rect_pos, rect_end, IM_COL32(30, 30, 30, 200), 3.0f);
                    draw_list->AddRect(rect_pos, rect_end, color, 3.0f);
                    draw_list->AddText(ImVec2(rect_pos.x + 5, rect_pos.y + 2), IM_COL32(255, 255, 255, 255), goto_text.c_str());
                } else if (force_bezier) { // Use bezier curves for specific cases like LOOK (currently unused)
                    ImVec2 cp1 = ImVec2(start_pos.x, start_pos.y + 50);
                    ImVec2 cp2 = ImVec2(end_pos.x, end_pos.y - 50);
                    draw_list->AddBezierCubic(start_pos, cp1, cp2, end_pos, color, 2.0f);
                } else { // Use straight lines for everything else (sequential flow and labeled branches).
                    draw_list->AddLine(start_pos, end_pos, color, 2.0f);
                    if (label) draw_list->AddText(ImVec2(start_pos.x + (end_pos.x-start_pos.x)*0.2f, start_pos.y + (end_pos.y-start_pos.y)*0.2f - 15), color, label);
                }

                // Draw a simple triangle arrow head at the end of the edge.
                ImVec2 dir = ImVec2(end_pos.x - start_pos.x, end_pos.y - start_pos.y);
                float len = sqrtf(dir.x*dir.x + dir.y*dir.y);
                if (len > 0) {
                    dir.x /= len; dir.y /= len;
                    ImVec2 p1 = ImVec2(end_pos.x - dir.x * 10 - dir.y * 4, end_pos.y - dir.y * 10 + dir.x * 4);
                    ImVec2 p2 = ImVec2(end_pos.x - dir.x * 10 + dir.y * 4, end_pos.y - dir.y * 10 - dir.x * 4);
                    draw_list->AddTriangleFilled(end_pos, p1, p2, color);
                }
            }
        };

        // Determine which edges to draw based on the instruction type.
        if (instruction == LOOK) {
            draw_edge((i + 1) % genome.size(), IM_COL32(200, 200, 200, 150), ""); // Gray for "empty"
            draw_edge((i + 2) % genome.size(), IM_COL32(0, 255, 0, 150), "");
            draw_edge((i + 3) % genome.size(), IM_COL32(0, 0, 255, 150), "");
        } else if (instruction >= JUMP_IF_EQUAL && instruction <= JUMP_IF_GREATER) {
            ImVec2 node_pos = ImVec2(p.x + node_positions[i].x, p.y + node_positions[i].y);
            ImVec2 node_size(200, 50);

            // Define start points for true (bottom) and false (right) paths
            ImVec2 true_start_pos = ImVec2(node_pos.x, node_pos.y + node_size.y * 0.5f);
            ImVec2 false_start_pos = ImVec2(node_pos.x + node_size.x, node_pos.y + node_size.y * 0.5f);

            int jump_offset = (i + 1 < genome.size()) ? (genome[i + 1] % 10) : 0;
            draw_edge((i + jump_offset) % genome.size(), IM_COL32(0, 255, 0, 200), "", false, &true_start_pos);
            draw_edge((i + 2) % genome.size(), IM_COL32(255, 0, 0, 200), "", false, &false_start_pos);
        } else if (instruction >= JUMP && instruction <= MAX_INSTRUCTION_VALUE) {
            int jump_offset = instruction;
            draw_edge((i + jump_offset) % genome.size(), IM_COL32(255, 255, 255, 200));
        } else {
            draw_edge((i + 1) % genome.size(), IM_COL32(255, 255, 255, 150), nullptr, false, &default_start_pos);
        }
    }

    // --- Pass 2: Draw Nodes ---
    // Nodes are drawn on top of the edges.
    for (int i = 0; i < genome.size(); ++i) {
        if (node_positions[i].x < 0) continue; // Skip non-reachable nodes

        ImVec2 node_pos = ImVec2(p.x + node_positions[i].x, p.y + node_positions[i].y);
        ImVec2 node_size(0, 0);

        const unsigned int instruction = genome[i];
        // Diamond shape for conditional nodes
        bool is_conditional = (instruction == LOOK ||
                               (instruction >= JUMP_IF_EQUAL && instruction <= JUMP_IF_GREATER));
        
        bool is_active = (i == current_pc);
        ImU32 node_bg_color = IM_COL32(50, 50, 50, 255); // Default BG

        // Draw the node shape (diamond for conditionals, rectangle for others).
        if (is_conditional) {
            node_size = ImVec2(200, 50);
            ImVec2 points[] = {
                ImVec2(node_pos.x + node_size.x * 0.5f, node_pos.y),
                ImVec2(node_pos.x + node_size.x, node_pos.y + node_size.y * 0.5f),
                ImVec2(node_pos.x + node_size.x * 0.5f, node_pos.y + node_size.y),
                ImVec2(node_pos.x, node_pos.y + node_size.y * 0.5f)
            };

            // Check if this node is a target of a conditional jump to apply a green/red tint,
            // but only if it's not a long jump (which is handled by GOTO coloring).
            for (int j = 0; j < genome.size(); ++j) {
                if (node_positions[j].x < 0) continue;
                unsigned int source_instr = genome[j];
                if (source_instr >= JUMP_IF_EQUAL && source_instr <= JUMP_IF_GREATER) {
                    int jump_offset = (j + 1 < genome.size()) ? (genome[j + 1] % 10) : 0;
                    int true_target = (j + jump_offset) % genome.size();
                    int false_target = (j + 2) % genome.size();

                    // Only color the node if it's NOT a long/backward jump (which will be a GOTO)
                    // This logic must exactly match the heuristic in `draw_edge`.
                    bool is_long_jump_true = (node_positions[true_target].y < node_positions[j].y - 100.0f) ||
                                             (std::abs(node_positions[true_target].x - node_positions[j].x) > 400.0f);
                    
                    bool is_long_jump_false = (node_positions[false_target].y < node_positions[j].y - 100.0f) ||
                                              (std::abs(node_positions[false_target].x - node_positions[j].x) > 400.0f);


                    if (i == true_target && !is_long_jump_true) node_bg_color = IM_COL32(0, 60, 0, 255);
                    else if (i == false_target && !is_long_jump_false) node_bg_color = IM_COL32(60, 0, 0, 255);
                }
            }

            draw_list->AddConvexPolyFilled(points, 4, node_bg_color);
            draw_list->AddPolyline(points, 4, IM_COL32(150, 150, 150, 255), ImDrawFlags_Closed, 1.0f);
        } else {
            node_size = ImVec2(200, 30); // Increased width for text
            draw_list->AddRectFilled(node_pos, ImVec2(node_pos.x + node_size.x, node_pos.y + node_size.y), node_bg_color, 5.0f);
            draw_list->AddRect(node_pos, ImVec2(node_pos.x + node_size.x, node_pos.y + node_size.y), IM_COL32(150, 150, 150, 255), 5.0f);
        }

        // The currently active node gets a special yellow highlight.
        if (is_active) draw_list->AddRect(node_pos, ImVec2(node_pos.x + node_size.x, node_pos.y + node_size.y), IM_COL32(255, 255, 0, 255), 5.0f, 0, 2.0f);

        // Draw the instruction text centered inside the node.
        std::string text = std::to_string(i) + ": " + getInstructionName(genome[i]);
        if (instruction >= JUMP && instruction <= MAX_INSTRUCTION_VALUE) {
            int jump_target = (i + instruction) % genome.size();
            text = std::to_string(i) + ": JMP [" + std::to_string(jump_target) + "]";
        } else {
            text = std::to_string(i) + ": " + getInstructionName(instruction);
        }
        if (instruction >= JUMP && instruction <= MAX_INSTRUCTION_VALUE) {
            int jump_target = (i + instruction) % genome.size();
            text = std::to_string(i) + ": JMP [" + std::to_string(jump_target) + "]";
        } else {
            text = std::to_string(i) + ": " + getInstructionName(instruction);
        }
        ImVec2 text_size = ImGui::CalcTextSize(text.c_str());
        ImVec2 text_pos = ImVec2(
            node_pos.x + (node_size.x - text_size.x) * 0.5f,
            node_pos.y + (node_size.y - text_size.y) * 0.5f
        );

        if (is_active) {
            draw_list->AddText(text_pos, IM_COL32(255, 255, 0, 255), text.c_str());
        } else {
            draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), text.c_str());
        }

        // Keep track of the graph's total size for scrolling.
        max_x = std::max(max_x, node_pos.x + node_size.x);
        max_y = std::max(max_y, node_pos.y + node_size.y);
    }

    // Create a dummy item with the total size of the graph to make the scrollbars work.
    ImGui::Dummy(ImVec2(max_x - p.x + 50.0f, max_y - p.y + 50.0f));
}

// Draws the panel showing the bot's internal state.
void GenomeAnalyzer::drawBotState() {
    ImGui::Text("Energy: %d", sim_bot->getEnergy());
    ImGui::Text("Age: %d", sim_bot->getAge());
    ImGui::Text("Position: (%.0f, %.0f)", sim_bot->getPosition().x, sim_bot->getPosition().y);
    ImGui::Text("Direction: %d", sim_bot->getDirection());
    ImGui::Text("PC: %d", sim_bot->getPC());

    ImGui::Separator();
    ImGui::Text("Memory Stack (top to bottom):");
    std::stack<unsigned int> mem_copy = sim_bot->getMemory();
    if (mem_copy.empty()) {
        ImGui::Text("<empty>");
    } else {
        while(!mem_copy.empty()) {
            ImGui::Text("%u", mem_copy.top());
            mem_copy.pop();
        }
    }
}

/** @brief Draws the buttons for placing new entities in the local simulation. */
void GenomeAnalyzer::drawPlacementControls() {
    ImGui::Spacing();
    // Helper lambda for creating toggle-like buttons for placement mode.
    auto placement_button = [&](const char* label, PlacementMode mode) {
        bool is_active = (current_placement_mode == mode);
        if (is_active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        if (ImGui::Button(is_active ? "Cancel" : label)) {
            current_placement_mode = is_active ? PLACE_NONE : mode;
        }
        if (is_active) ImGui::PopStyleColor();
    };

    placement_button("Add Empty Bot", PLACE_EMPTY_BOT); ImGui::SameLine();
    placement_button("Add Relative", PLACE_RELATIVE); ImGui::SameLine();
    placement_button("Add Organic", PLACE_ORGANIC); ImGui::SameLine();
    placement_button("Remove", PLACE_REMOVE);
    ImGui::Spacing();
}

// Draws the mini-world visualization.
void GenomeAnalyzer::drawBotVisualization() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Calculate cell size and grid position to fit within the panel.
    float cell_vis_size = std::min(bot_vis_size.x / LOCAL_WORLD_SIZE, bot_vis_size.y / LOCAL_WORLD_SIZE) * 0.9f;
    ImVec2 grid_top_left = ImVec2(
        bot_vis_pos.x + (bot_vis_size.x - cell_vis_size * LOCAL_WORLD_SIZE) * 0.5f,
        bot_vis_pos.y + (bot_vis_size.y - cell_vis_size * LOCAL_WORLD_SIZE) * 0.5f
    );

    // Draw the grid lines.
    for (int i = 0; i <= LOCAL_WORLD_SIZE; ++i) {
        draw_list->AddLine(ImVec2(grid_top_left.x + i * cell_vis_size, grid_top_left.y), ImVec2(grid_top_left.x + i * cell_vis_size, grid_top_left.y + LOCAL_WORLD_SIZE * cell_vis_size), IM_COL32(100, 100, 100, 255));
        draw_list->AddLine(ImVec2(grid_top_left.x, grid_top_left.y + i * cell_vis_size), ImVec2(grid_top_left.x + LOCAL_WORLD_SIZE * cell_vis_size, grid_top_left.y + i * cell_vis_size), IM_COL32(100, 100, 100, 255));
    }

    // Draw all entities (bots, organic matter) in the local world.
    const auto& bots = local_world->getBots();
    for (Bot* bot : bots) {
        if (!bot) continue;
        if (bot->is_dead) continue;

        // Calculate the screen position for the bot's cell.
        Vector2 pos = bot->getPosition();
        ImVec2 cell_top_left = ImVec2(grid_top_left.x + pos.x * cell_vis_size, grid_top_left.y + pos.y * cell_vis_size);
        
        if (bot->isOrganic) {
            draw_list->AddRectFilled(cell_top_left, ImVec2(cell_top_left.x + cell_vis_size, cell_top_left.y + cell_vis_size), IM_COL32(128, 128, 128, 255));
        } else {
            Color c = bot->getColor();
            draw_list->AddRectFilled(cell_top_left, ImVec2(cell_top_left.x + cell_vis_size, cell_top_left.y + cell_vis_size), IM_COL32(c.r, c.g, c.b, c.a));

            // If this is the main bot being analyzed, draw a white border to highlight it.
            if (bot == sim_bot) {
                draw_list->AddRect(cell_top_left, ImVec2(cell_top_left.x + cell_vis_size, cell_top_left.y + cell_vis_size), IM_COL32(255, 255, 255, 255), 0.0f, 0, 2.0f);
            }

            // Draw a yellow line to indicate the bot's current direction.
            ImVec2 center = ImVec2(cell_top_left.x + cell_vis_size * 0.5f, cell_top_left.y + cell_vis_size * 0.5f);
            Vector2 dir_vecs[] = {{-1,-1},{0,-1},{1,-1},{1,0},{1,1},{0,1},{-1,1},{-1,0}};
            Vector2 dir_vec = dir_vecs[bot->getDirection()];
            ImVec2 end_point = ImVec2(center.x + dir_vec.x * cell_vis_size * 0.4f, center.y + dir_vec.y * cell_vis_size * 0.4f);
            draw_list->AddLine(center, end_point, IM_COL32(255, 255, 0, 255), 2.0f);
        }
    }

    // --- Placement Mode Logic ---
    if (current_placement_mode != PLACE_NONE && ImGui::IsWindowHovered()) {
        ImVec2 mouse_pos = ImGui::GetMousePos();
        if (mouse_pos.x >= grid_top_left.x && mouse_pos.y >= grid_top_left.y &&
            mouse_pos.x < grid_top_left.x + LOCAL_WORLD_SIZE * cell_vis_size &&
            mouse_pos.y < grid_top_left.y + LOCAL_WORLD_SIZE * cell_vis_size)
        {
            int grid_x = (int)((mouse_pos.x - grid_top_left.x) / cell_vis_size);
            int grid_y = (int)((mouse_pos.y - grid_top_left.y) / cell_vis_size);
            Vector2 target_pos = {(float)grid_x, (float)grid_y};

            // Draw preview
            ImVec2 cell_top_left = ImVec2(grid_top_left.x + grid_x * cell_vis_size, grid_top_left.y + grid_y * cell_vis_size);
            ImU32 preview_color = IM_COL32(255, 255, 255, 100);
            if (current_placement_mode == PLACE_ORGANIC) {
                preview_color = IM_COL32(128, 128, 128, 100);
            } else if (current_placement_mode == PLACE_RELATIVE) {
                Color c = sim_bot->getColor();
                preview_color = IM_COL32(c.r, c.g, c.b, 100);
            } else if (current_placement_mode == PLACE_REMOVE) {
                preview_color = IM_COL32(255, 0, 0, 100);
            }
            draw_list->AddRectFilled(cell_top_left, ImVec2(cell_top_left.x + cell_vis_size, cell_top_left.y + cell_vis_size), preview_color);

            // Place on click
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                Bot* bot_at_target = local_world->getBotAt(target_pos);
                if (current_placement_mode == PLACE_REMOVE) {
                    if (bot_at_target != nullptr && bot_at_target != sim_bot) { // Don't allow removing the main bot
                        local_world->removeBot(bot_at_target);
                        // If we removed a bot that just reproduced, its child might be what we clicked on.
                        // The main sim_bot could be dead but not yet null.
                        if (bot_at_target->is_dead) {
                            // bot_at_target is now a dangling pointer, but we don't use it again.
                        }
                    }
                } else if (bot_at_target == nullptr) {
                    Bot* new_bot = nullptr;
                    switch (current_placement_mode) {
                        case PLACE_EMPTY_BOT: {
                            new_bot = new Bot();
                            auto& genome = const_cast<std::vector<unsigned int>&>(new_bot->getGenome());
                            genome.clear();
                            genome.push_back(PHOTOSYNTHIZE);
                            break;
                        }
                        case PLACE_RELATIVE:
                            new_bot = new Bot(*sim_bot); // Create a copy
                            break;
                        case PLACE_ORGANIC:
                            new_bot = new Bot();
                            new_bot->isOrganic = true;
                            new_bot->addEnergy(50); // Give it some energy to be worth eating
                            break;
                        default: break;
                    }

                    if (new_bot) {
                        new_bot->setPosition(target_pos);
                        local_world->addBot(new_bot);
                    }
                }
            }
        }
    }
}
