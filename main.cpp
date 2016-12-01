#include <iostream>
#include <array>
#include <vector>
#include <fstream>
#include <streambuf>
#include <string>
#include <exception>
#include <set>
#include <ctime>
#include <map>
#include <memory>

#include <rl>

#include "analytic-engine-cli.h"

using namespace std;
using json = nlohmann::json;

const string appName = "analytic-engine-cli";

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
  size_t goalPatternTimeBegin = configJSON["goalPattern"]["timeBegin"];
  size_t goalPatternTimeEnd = configJSON["goalPattern"]["timeEnd"];
  size_t iterationCount = configJSON["iterationCount"];
  float initialReward = configJSON["initialReward"];
  float stepSize = configJSON["reinforcementLearning"]["stepSize"];
  float discountRate = configJSON["reinforcementLearning"]["discountRate"];
  string resultFile = configJSON["resultFile"];

  auto metrics = Metric::parseMetrics(metricSJSON);

  auto minMaxMetricTime = Metric::getMinMaxTime(metrics);

  std::cout << "Min metric time: " << minMaxMetricTime.first << std::endl;
  std::cout << "Max metric time: " << minMaxMetricTime.second << std::endl;
  std::cout << "Metric time duration (max metric time - min metric time): "
            << (minMaxMetricTime.second - minMaxMetricTime.first) / 60.0F
            << "min"
            << std::endl;

  // todo: make these cli arg.
  size_t goalPatternTimeDuration = goalPatternTimeEnd - goalPatternTimeBegin;
  std::cout << "Min pattern time: " << goalPatternTimeBegin << std::endl;
  std::cout << "Max pattern time: " << goalPatternTimeEnd << std::endl;
  std::cout << "Goal pattern duration (Max pattern time - Min pattern time): "
            << goalPatternTimeDuration << std::endl;

  vector<rl::spState<STATE>> patterns =
      Metric::getPatternsFromMetrics<ANALYTIC_ENGINE::PATTERN_SIZE>(
          metrics,
          goalPatternTimeBegin,
          goalPatternTimeEnd);

  // Since Metric::getPatternsFromMetrics filters out metrics that can't span
  // the whole [goalPatternTimeBegin, goalPatternTimeEnd], thus we can acquire
  // a list of filtered metrics from this.
  decltype(metrics) filteredMetrics;
  for (auto p : patterns) {
    filteredMetrics.push_back(p->getMetric());
  }

  std::cerr << "Metric count with invalid resolution: "
            << metrics.size() - patterns.size()
            << std::endl;
  std::cout << "Valid metrics: "
            << patterns.size()
            << std::endl;

  size_t goalPatternIndex = 0;
  PlotPatternSpecialized::getPatternIndexFromMetricName(
      patterns,
      goalMetric,
      goalPatternIndex);

  if (goalPatternIndex == patterns.size()) {
    std::cerr << "Goal Pattern was not found in the given metrics." << std::endl;
    return 1;
  }

  auto goalState = patterns[goalPatternIndex];

  // Setup policy.
  rl::policy::EpsilonGreedy<rl::floatVector, rl::floatVector> policy(1.0F);
  // Setup tile coding.
  vector <rl::coding::DimensionInfo<rl::FLOAT>> dimensionalInfoVector = {
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 1.0F, 10),  // y1
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 1.0F, 10),  // y2
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 1.0F, 10),  // y3
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 1.0F, 10),  // y4
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 1.0F, 10),  // y5
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 1.0F, 10),  // y6
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 1.0F, 10),  // y7
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 1.0F, 10),  // y8
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 1.0F, 10),  // y9
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 1.0F, 10),  // y10
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 11042.0F, 11043, 0.0F),  // Metrics that will lead to goalState.
      rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 0.0F, 1, 0.0F),  // Metrics that will lead to goalState.
      //rl::coding::DimensionInfo<rl::FLOAT>(0.0F, 11042.0F, 11043, 0.0F),  // Metrics that will lead to goalState.
  };

  std::cout << "Allocating Memory." << std::endl;
  rl::coding::TileCodeMurMur tileCode(dimensionalInfoVector, 10, 600000000);  // Setup tile coding with 10 offsets.
  rl::algorithm::QLearningETGD qLearning(tileCode, stepSize, discountRate, 0.9F, policy);
  std::cout << "Finished Allocating Memory." << std::endl;

  rl::spActionSet<rl::floatVector> actions({ ANALYTIC_ENGINE::goalAction });
  auto actionSet = rl::ActionSet<rl::floatVector>(actions);
  qLearning.setDefaultStateActionValue(initialReward);
  rl::AgentSupervised<rl::floatVector, rl::floatVector> agent(actionSet, qLearning);

  app::train(iterationCount,
             filteredMetrics,
             goalState,
             agent,
             minMaxMetricTime.first,
             minMaxMetricTime.second);

  /*auto rewardMultimap = qLearning.getStateActionPairContainer().getReverseMap();*/

  // Get the reward for each metrics.
  std::multimap<rl::FLOAT, rl::StateAction<STATE, ACTION>> rewardMap;
  for (auto p : patterns) {
    auto reward =  qLearning.getStateActionValue(rl::StateAction<rl::floatVector, rl::floatVector>(
        p->getGradientDescentParameters(), ANALYTIC_ENGINE::goalAction));
    rewardMap.insert(std::pair<rl::FLOAT, rl::StateAction<STATE, ACTION>>(
        reward, rl::StateAction<STATE, ACTION>(p, goalState)
    ));
  }

  app::serializeResult(resultFile, rewardMap);

  return 0;
}