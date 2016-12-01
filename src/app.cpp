//
// Created by jandres on 11/11/16.
//

#include <random>
#include <iostream>
#include <fstream>

#include <rl>

#include "app.h"
#include "declares.h"
#include "plot-pattern.h"
#include "metric.h"
#include "../lib/json.hpp"

using json = nlohmann::json;

namespace app {

void train(size_t iterationCount,
           const vector<std::shared_ptr<Metric>> &metrics,
           shared_ptr<STATE> &goalState,
           rl::AgentSupervised<rl::floatVector, rl::floatVector> &agent,
           size_t minMetricTime,
           size_t maxMetricTime) {

  size_t goalPatternTimeBegin = goalState->getTimeBegin();
  size_t goalPatternTimeEnd = goalState->getTimeEnd();
  size_t goalPatternTimeDuration = goalPatternTimeEnd - goalPatternTimeBegin;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(
      minMetricTime,
      maxMetricTime - goalPatternTimeDuration);

  for (size_t i = 0; i < iterationCount; i++) {
    app::time patternTimeBegin = dis(gen);
    app::time patternTimeEnd = patternTimeBegin + goalPatternTimeDuration;

    auto goalMetric = goalState->getMetric();

    rl::spState<STATE> currentGoalPattern;
    try {
      currentGoalPattern = Metric::getPattern<app::PATTERN_SIZE>(
          goalMetric,
          patternTimeBegin,
          patternTimeEnd);
    } catch(...) {
      // Don't iterate if the goal metric don't have a metric for this time frame.
      continue;
    }

    for (auto metric : metrics) {
      auto currentPattern = Metric::getPattern<app::PATTERN_SIZE>(
          metric,
          patternTimeBegin,
          patternTimeEnd);

      agent.train(
          currentPattern->getGradientDescentParameters(),
          app::goalAction,
          -goalState->getAbsoluteArea(*currentGoalPattern),  // Reward.
          goalState->getGradientDescentParameters());
    }

    std::cout << "Traning: "
              << (static_cast<float>(i) / static_cast<float>(iterationCount)) * 100.0f
              << "%"
              << std::endl;
  }
}

void serializeResult(const string &resultFile,
                     const multimap<rl::FLOAT, rl::StateAction<STATE, ACTION>> &rewardMultimap) {
  json resultJSON;
  set<string> metricNameSet;
  for (auto rewardStatePair : rewardMultimap) {
    auto pattern = rewardStatePair.second.getState();
    auto entailedPattern = rewardStatePair.second.getAction();

    string metricName = pattern->getMetricName();
    string entailedMetricName = entailedPattern->getMetricName();
    double reward = rewardStatePair.first;

    json entry = {
        {
            "sourcePattern" ,
            {
                { "metric", metricName },
                { "timeBegin", static_cast<size_t>(pattern->getTimeBegin()) },
                { "timeEnd", static_cast<size_t>(pattern->getTimeEnd()) }
            }
        },
        {
            "destPattern",
            {
                { "metric", entailedMetricName },
                { "timeBegin", static_cast<size_t>(pattern->getTimeBegin()) },
                { "timeEnd", static_cast<size_t>(pattern->getTimeEnd()) }
            }
        },
        { "reward", reward },
    };
    resultJSON.push_back(entry);
    metricNameSet.insert(metricName);
  }

  cout << "Trained metric count: " << metricNameSet.size() << endl;

  ofstream resultFileStream;
  resultFileStream.open(resultFile);
  resultFileStream << resultJSON.dump(2);
  resultFileStream.close();
}

}  // namespace APP