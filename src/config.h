#pragma once
#define WORLD_WIDTH 165
#define WORLD_HEIGHT 100

#define CELL_SIZE 16
#define BG_COLOR RAYWHITE
#define GRID_THICKNESS 1
#define GRID_COLOR BLACK

#define TOP_PANEL_HEIGHT CELL_SIZE*2

#define MUTATION_RATE 0.01f // Chance for a single gene to mutate
#define GENOME_INSERTION_RATE 0.01f // Chance to add a gene
#define GENOME_DELETION_RATE 0.01f // Chance to remove a gene

#define BOTTOM_PANEL_HEIGHT 50
#define SIDE_PANEL_WIDTH 400

#define INITIAL_GENOME_SIZE 64
#define MIN_GENOME_SIZE 4
#define MAX_GENOME_SIZE 2048
#define MEMORY_SIZE 64

#define INITIAL_ENERGY 150
#define REPRODUCTION_ENERGY_MINIMUM 100
#define MAX_ENERGY 300

#define PHOTOSYNTHIZE_ENERGY_GAIN 10 // Used in middle biome
#define HIGH_PHOTOSYNTHIZE_ENERGY_GAIN 20 // Used in left biome
#define LOW_PHOTOSYNTHIZE_ENERGY_GAIN 1 // Used in right biome

#define COLOR_MUTATION_AMOUNT 20

#define MAXIMUM_BOT_AGE 3000