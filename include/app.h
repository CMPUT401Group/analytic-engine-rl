//
// Created by jandres on 11/11/16.
//

#pragma once

#include <string>
#include <memory>

#include "declares.h"
#include "plot-pattern.h"

namespace app {

/**
 * Training the agent's model to predict the goal Metric. Basically giving the agent a way to tell
 * what set of metrics will likely lead to goalState A.
 *
 * @param iterationCount Number of iteration. The higher the better the closer is the resulting model to reality.
 * @param metrics A list of graphite metrics.
 * @param goalState A shared_ptr to a plot-pattern.
 * @param agent The agent that will be trained/
 * @param minMetricTime Minimum time to train (unix time stamp).
 * @param maxMetricTime Maximum time to train (unix time stamp).
 */
void train(size_t iterationCount,
           const vector<std::shared_ptr<Metric>> &metrics,
           rl::spState<STATE> &goalState,
           rl::AgentSupervised<STATE, ACTION> &agent,
           size_t minMetricTime,
           size_t maxMetricTime);

/**
 * Serialize the model (represented by reverse multimap) to a json file.
 * @param resultFile The file to which te result will be dumped.
 * @param rewardMultimap A mapping of reward (sorted from least to greatest) to
 *                      their associated state-action pair.
 */
void serializeResult(const string &resultFile,
                     const multimap<rl::FLOAT, rl::StateAction<STATE, ACTION>> &rewardMultimap);

}  // namespace APP