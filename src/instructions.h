#pragma once

enum Instruction {
    // Actions
    MOVE = 0,
    TURN,
    LOOK,
    ATTACK,
    PHOTOSYNTHIZE,
    CHECK_RELATIVE,
    SHARE_ENERGY,
    CONSUME_ORGANIC,
    REPRODUCE,
    JUMP_UNCONDITIONAL,

    // Checks
    CHECK_BIOME,
    CHECK_X,
    CHECK_Y,
    CHECK_ENERGY,
    CHECK_AGE,

    // Control Flow
    JUMP_IF_EQUAL,
    JUMP_IF_NOT_EQUAL,
    JUMP_IF_GREATER
};

const int MAX_INSTRUCTION_VALUE = 127;
