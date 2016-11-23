//
// Created by jandres on 11/11/16.
//

#pragma once

#include <string>
#include <memory>

#include "declares.h"
#include "plot-pattern-state.h"

namespace app {

void train(size_t iterationCount,
           const vector<std::shared_ptr<Metric>> &metrics,
           rl::spState<STATE> &goalState,
           rl::AgentSupervised<STATE, ACTION> &agent,
           size_t minMetricTime,
           size_t maxMetricTime);

void serializeResult(const string &resultFile,
                     const multimap<rl::FLOAT, rl::StateAction<STATE, ACTION>> &rewardMultimap);

}  // namespace APP