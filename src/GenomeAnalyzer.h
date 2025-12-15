#pragma once

#include "imgui.h"
#include "bot.h"
#include "world.h"

/**
 * @class GenomeAnalyzer
 * @brief A debugging tool that provides an interactive, visual representation of a bot's genome.
 *
 * This class creates a separate, local simulation environment to analyze a single bot's
 * behavior step-by-step. It displays the bot's genome as a control-flow graph,
 * visualizes the bot's actions in a mini-world, and shows its internal state (energy, memory, etc.).
 * The main simulation is paused while the analyzer is open.
 */
class GenomeAnalyzer {
public:
    /**
     * @brief Constructs a new GenomeAnalyzer instance.
     */
    GenomeAnalyzer();
    /**
     * @brief Destroys the GenomeAnalyzer instance, cleaning up allocated resources.
     */
    ~GenomeAnalyzer();

    /**
     * @brief Opens the analyzer window for a specific bot.
     * @param bot A pointer to the bot to be analyzed.
     */
    void analyze(Bot* bot);
    /**
     * @brief Draws the Genome Analyzer window and all its components.
     * This is the main entry point to be called in the UI loop.
     */
    void draw();
    /**
     * @brief Checks if the analyzer window is currently open.
     * @return True if the window is open, false otherwise.
     */
    bool isOpen() const;
    /**
     * @brief Closes the analyzer window and cleans up the local simulation.
     */
    void close();

private:
    /**
     * @brief Resets the local simulation to its initial state.
     * Creates a new copy of the bot and a new local world.
     */
    void resetSimulation();
    /**
     * @brief Builds the layout for the genome graph.
     * Performs a reachability analysis to determine node positions for a clear flow.
     */
    void buildGraphLayout();
    /**
     * @brief Advances the local simulation by a single step.
     */
    void step();

    // --- Constants ---
    static const int LOCAL_WORLD_SIZE = 11; ///< The width and height of the local simulation grid.

    // --- State ---
    bool is_open = false;       ///< Flag indicating if the analyzer window is visible.
    bool is_paused = true;      ///< Flag indicating if the local simulation is paused.

    Bot* original_bot = nullptr; ///< A pointer to the bot in the main simulation being analyzed.
    Bot* sim_bot = nullptr;      ///< A deep copy of the original bot, used for the local simulation.
    World* local_world = nullptr;///< A small, self-contained world for the local simulation.

    // --- UI and Visualization Data ---
    ImVec2 bot_vis_pos;  ///< Screen position of the top-left corner of the bot visualization panel.
    ImVec2 bot_vis_size; ///< Size of the bot visualization panel.

    // --- Placement Mode ---
    enum PlacementMode {
        PLACE_NONE,
        PLACE_EMPTY_BOT,
        PLACE_RELATIVE,
        PLACE_ORGANIC,
        PLACE_REMOVE
    };
    PlacementMode current_placement_mode = PLACE_NONE;

    std::vector<ImVec2> node_positions; ///< Stores the calculated screen positions for each node in the genome graph.

    // --- Drawing Sub-routines ---
    /** @brief Draws the interactive control-flow graph of the bot's genome. */
    void drawGenomeGraph();
    /** @brief Draws the current state of the simulated bot (energy, memory, etc.). */
    void drawBotState();
    /** @brief Draws the control buttons (Run, Pause, Step, Reset). */
    void drawControls();
    /** @brief Draws the mini-world visualization showing the bot's actions. */
    void drawBotVisualization();
    /** @brief Draws the buttons for placing new entities in the local simulation. */
    void drawPlacementControls();
};
