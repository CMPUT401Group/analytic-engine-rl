#include <iostream>
#include <array>
#include <vector>
#include <fstream>
#include <streambuf>
#include <string>
#include <exception>

#include <rl>

#include "analytic-engine-cli.h"

const string appName = "analytic-engine-cli";

using namespace std;
using json = nlohmann::json;

const size_t PATTERN_SIZE = 15;
using STATE = PlotPatternState<PATTERN_SIZE>;
using ACTION = PlotPatternState<PATTERN_SIZE>;

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cerr << ("Terminal format is \"./" + appName + " <*.json> <*.json>\".") << std::endl;
    exit(1);
  }

  std::string metricsFileName(argv[1]);
  std::string configFileName(argv[2]);

  std::ifstream metricsFileStream(metricsFileName);
  if (!metricsFileStream.is_open()) {
    std::cerr << "Problem opening metric file. Probably does not exist." << std::endl;
    exit(1);
  }

  std::ifstream configFileStream(configFileName);
  if (!configFileStream.is_open()) {
    std::cerr << "Problem opening config file. Probably does not exist." << std::endl;
    exit(1);
  }

  std::string metricSFileString((std::istreambuf_iterator<char>(metricsFileStream)), std::istreambuf_iterator<char>());
  auto metricSJSON = json::parse(metricSFileString);

  std::string configFileString((std::istreambuf_iterator<char>(configFileStream)), std::istreambuf_iterator<char>());
  auto configJSON = json::parse(configFileString);

  string goalMetric = configJSON["goalMetric"];
  size_t patternTimeBegin = configJSON["goalPatternTimeBegin"];
  size_t patternTimeEnd = configJSON["goalPatternTimeEnd"];
  size_t scanStepSize = configJSON["scanStepSize"];
  float initialReward = configJSON["initialReward"];
  float stepSize = configJSON["reinforcementLearning"]["stepSize"];
  float discountRate = configJSON["reinforcementLearning"]["discountRate"];
  string resultFile = configJSON["resultFile"];

  std::vector<Metric*> metrics = Metric::parseMetrics(metricSJSON);

  auto minMaxMetricTime = Metric::getMinMaxTime(metrics);

  std::cout << "Min metric time: " << minMaxMetricTime.first << std::endl;
  std::cout << "Max metric time: " << minMaxMetricTime.second << std::endl;
  std::cout << "Metric time duration (max metric time - min metric time): "
            << (minMaxMetricTime.first - minMaxMetricTime.second) / 60.0F
            << "min"
            << std::endl;

  // todo: make these cli arg.
  size_t patternDuration = patternTimeEnd - patternTimeBegin;
  std::cout << "Min pattern time: " << patternTimeBegin << std::endl;
  std::cout << "Max pattern time: " << patternTimeEnd << std::endl;
  std::cout << "Pattern duration (Max pattern time - Min pattern time): " << patternDuration << std::endl;

  vector<PlotPattern<PATTERN_SIZE>> patterns = Metric::getPatternsFromMetrics<PATTERN_SIZE>(
      metrics,
      patternTimeBegin,
      patternTimeEnd);

  std::cerr << "Invalid resolution: " << metrics.size() - patterns.size() << std::endl;
  std::cout << "Extracted patterns: " << patterns.size() << std::endl;

  vector<STATE> patternStates = STATE::getPlotPatternStatesFromPatterns(patterns);

  size_t goalPatternIndex = 0;
  for (; goalPatternIndex < patterns.size(); goalPatternIndex++) {
    if (patterns[goalPatternIndex].getMetricName() == goalMetric) {
      std::cout << "Goal state located" << std::endl;
      break;
    }
  }

  if (goalPatternIndex == patterns.size()) {
    std::cerr << "Goal Pattern was not found in the given metrics." << std::endl;
    return 1;
  }

  PlotPattern<PATTERN_SIZE> goalPattern = patterns[goalPatternIndex];
  STATE goalState(goalPattern);

  AnalyticEngineEnvironment<STATE, ACTION> aee;

  AI::ActuatorBase<STATE, ACTION> actuator(aee);
  for (auto p : patternStates) { actuator.addAction(p); }

  AI::Algorithm::Policy::EpsilonGreedy<STATE, ACTION> policy(1.0F);
  AI::Algorithm::RL::Sarsa<STATE, ACTION> sarsaAlgorithm(stepSize, discountRate, policy);
  sarsaAlgorithm.setDefaultStateActionValue(initialReward);

  AI::AgentSupervised<STATE, ACTION> agent(actuator, sarsaAlgorithm);

  size_t iterationCount = 0;
  size_t maximumIterationCount = ((minMaxMetricTime.second - minMaxMetricTime.first)/scanStepSize) * patternStates.size();
  size_t updateIteration = maximumIterationCount * 0.01;  // Update every 1%.
  for (size_t time = minMaxMetricTime.first; time < minMaxMetricTime.second - patternDuration; time+=scanStepSize) {
    ANALYTIC_ENGINE::time patternTimeBegin = time;
    ANALYTIC_ENGINE::time patternTimeEnd = patternTimeBegin + patternDuration;

    auto goalMetric = goalPattern.getMetric();
    auto currentGoalPattern = goalMetric.getPattern<PATTERN_SIZE>(patternTimeBegin, patternTimeEnd);

    for (auto patternState : patternStates) {
      auto pattern = patternState.getValue();
      auto metric = pattern.getMetric();

      try {
        auto currentPattern = metric.getPattern<PATTERN_SIZE>(patternTimeBegin, patternTimeEnd);
        if (pattern == currentPattern) {
          agent.train(
              patternState,
              goalState,
              -goalPattern.getAbsoluteArea(currentGoalPattern),  // Reward.
              goalState
          );
        }
      } catch(...) {
        //std::cerr << "time out of range in current metric" << std::endl;
      }

      if ((iterationCount++ % updateIteration) == 0) {
        std::cout << "Traning: "
                  << std::ceil(100 * static_cast<float>(iterationCount)/maximumIterationCount)
                  << "%"
                  << std::endl;
      }
    }
  }

  std::cout << "Iteration count: " << iterationCount << std::endl;

  auto rewardMultimap = sarsaAlgorithm.getStateActionPairContainer().getReverseMap();

  std::cout << std::endl << "Results (Organized from most entailment to least entailment): " << std::endl;
  json resultJSON;
  for (auto rewardStatePair : rewardMultimap) {
    string metricName = rewardStatePair.second.getState().getValue().getMetricName();
    double reward = rewardStatePair.first;
    resultJSON.push_back({ metricName, reward });
    std::cout << "Metric: " << metricName << ", reward: " << reward << std::endl;
  }

  std::ofstream resultFileStream;
  resultFileStream.open(resultFile);
  resultFileStream << resultJSON.dump(2);
  resultFileStream.close();

  // Garbage collection.
  for (auto m : metrics) {
    delete m;
    m = nullptr;
  }
  metrics.clear();

  return 0;
}