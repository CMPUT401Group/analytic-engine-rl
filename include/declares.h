//
// Created by jandres on 07/11/16.
//
#pragma once

#include <utility>
#include <rl>

namespace app {
const size_t PATTERN_SIZE = 10;
using time = size_t;
using point = std::pair<float, time>;

// Temporary. This represents the goal action.
// TODO(jandres): remove for the future when actions are actually metrics.
extern rl::spFloatVector goalAction;
}
