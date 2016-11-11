#include <iostream>
#include <array>
#include <vector>
#include <fstream>
#include <streambuf>
#include <string>
#include <exception>
#include <set>
#include <random>
#include <ctime>

#include <rl>

#include "analytic-engine-cli.h"

const string appName = "analytic-engine-cli";

using namespace std;
using json = nlohmann::json;

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

  string goalMetric = configJSON["goalPattern"]["metric"];
  size_t patternTimeBegin = configJSON["goalPattern"]["timeBegin"];
  size_t patternTimeEnd = configJSON["goalPattern"]["timeEnd"];
  size_t iterationCount = configJSON["iterationCount"];
  float initialReward = configJSON["initialReward"];
  float stepSize = configJSON["reinforcementLearning"]["stepSize"];
  float discountRate = configJSON["reinforcementLearning"]["discountRate"];
  string resultFile = configJSON["resultFile"];

  std::vector<Metric*> metrics = Metric::parseMetrics(metricSJSON);

  auto minMaxMetricTime = Metric::getMinMaxTime(metrics);

  std::cout << "Min metric time: " << minMaxMetricTime.first << std::endl;
  std::cout << "Max metric time: " << minMaxMetricTime.second << std::endl;
  std::cout << "Metric time duration (max metric time - min metric time): "
            << (minMaxMetricTime.second - minMaxMetricTime.first) / 60.0F
            << "min"
            << std::endl;

  // todo: make these cli arg.
  size_t patternDuration = patternTimeEnd - patternTimeBegin;
  std::cout << "Min pattern time: " << patternTimeBegin << std::endl;
  std::cout << "Max pattern time: " << patternTimeEnd << std::endl;
  std::cout << "Pattern duration (Max pattern time - Min pattern time): " << patternDuration << std::endl;

  vector<PlotPatternSpecialized> patterns = Metric::getPatternsFromMetrics<ANALYTIC_ENGINE::PATTERN_SIZE>(
      metrics,
      patternTimeBegin,
      patternTimeEnd);

  std::cerr << "Invalid resolution: " << metrics.size() - patterns.size() << std::endl;
  std::cout << "Extracted patterns: " << patterns.size() << std::endl;

  vector<STATE> patternStates = STATE::getPlotPatternStatesFromPatterns(patterns);

  size_t goalPatternIndex = 0;
  PlotPatternSpecialized::getPatternIndexFromMetricName(patterns, goalMetric, goalPatternIndex);

  if (goalPatternIndex == patterns.size()) {
    std::cerr << "Goal Pattern was not found in the given metrics." << std::endl;
    return 1;
  }

  PlotPatternSpecialized goalPattern = patterns[goalPatternIndex];
  STATE goalState(goalPattern);

  AnalyticEngineEnvironmentSpecialized aee;

  ActuatorBaseSpecialized actuator(aee);
  for (auto p : patternStates) { actuator.addAction(p); }

  AI::Algorithm::Policy::EpsilonGreedy<STATE, ACTION> policy(1.0F);
  AI::Algorithm::RL::Sarsa<STATE, ACTION> sarsaAlgorithm(stepSize, discountRate, policy);
  sarsaAlgorithm.setDefaultStateActionValue(initialReward);

  AI::AgentSupervised<STATE, ACTION> agent(actuator, sarsaAlgorithm);

  size_t maximumIterationCount = iterationCount * patternStates.size();
  size_t updateIteration = maximumIterationCount * 0.01;  // Update every 1%.

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(minMaxMetricTime.first, minMaxMetricTime.second - patternDuration);

  for (size_t i = 0; i < iterationCount; i++) {
    ANALYTIC_ENGINE::time patternTimeBegin = dis(gen);
    ANALYTIC_ENGINE::time patternTimeEnd = patternTimeBegin + patternDuration;

    auto goalMetric = goalPattern.getMetric();
    auto currentGoalPattern = goalMetric.getPattern<ANALYTIC_ENGINE::PATTERN_SIZE>(patternTimeBegin, patternTimeEnd);

    for (auto patternState : patternStates) {
      auto pattern = patternState.getValue();
      auto metric = pattern.getMetric();

      auto currentPattern = metric.getPattern<ANALYTIC_ENGINE::PATTERN_SIZE>(patternTimeBegin, patternTimeEnd);
      if (pattern == currentPattern) {
        agent.train(
            patternState,
            goalState,
            -goalPattern.getAbsoluteArea(currentGoalPattern),  // Reward.
            goalState
        );
      }
    }

    std::cout << "Traning: "
              << std::ceil((i * patternStates.size()) / updateIteration)
              << "%"
              << std::endl;
  }

  std::cout << "Iteration count: " << iterationCount << std::endl;

  auto rewardMultimap = sarsaAlgorithm.getStateActionPairContainer().getReverseMap();

  //std::cout << std::endl << "Results (Organized from most entailment to least entailment): " << std::endl;
  json resultJSON;
  set<string> metricNameSet;
  for (auto rewardStatePair : rewardMultimap) {
    auto pattern = rewardStatePair.second.getState().getValue();
    auto entailedPattern = rewardStatePair.second.getAction().getValue();

    string metricName = pattern.getMetricName();
    string entailedMetricName = entailedPattern.getMetricName();
    double reward = rewardStatePair.first;

    json entry = {
        {
            "sourcePattern" ,
            {
                { "metric", metricName },
                { "timeBegin", static_cast<size_t>(pattern.getTimeBegin()) },
                { "timeEnd", static_cast<size_t>(pattern.getTimeBegin()) }
            }
        },
        {
            "destPattern",
            {
                { "metric", entailedMetricName },
                { "timeBegin", static_cast<size_t>(pattern.getTimeBegin()) },
                { "timeEnd", static_cast<size_t>(pattern.getTimeBegin()) }
            }
        },
        { "reward", reward },
    };
    resultJSON.push_back(entry);
    metricNameSet.insert(metricName);
    //std::cout << "Metric: " << metricName << ", reward: " << reward << std::endl;
  }

  std::cout << "Trained metric count: " << metricNameSet.size() << std::endl;

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