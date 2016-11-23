#include <vector>
#include <string>
#include <array>

#include <rl>

#include "catch.hpp"
#include "analytic-engine-rl.h"

using std::vector;
using std::string;
using std::array;
using std::pair;
using std::make_pair;

using ANALYTIC_ENGINE::point;

using p = point;

std::map<string, array<point, 10>> TEST_METRICS {
    {
        "target-00",
        {
            p{0.0f, 1}, p{0.02f, 2}, p{0.11f, 3}, p{0.11f, 4}, p{-0.2f, 5}, p{0.0, 6}, p{0.03, 7}, p{0.11f, 8}, p{0.11f, 9}, p{-0.1f, 10}
        }
    },
    {
        "target-01",
        {
            p{3.0f, 1}, p{2.9f, 2}, p{1.0f, 3}, p{0.2f, 4}, p{-0.2f, 5}, p{3.0f, 6}, p{2.9f, 7}, p{0.9f, 8}, p{0.1f, 9}, p{-0.1f, 10}
        }
    },
    {
        "target-02",
        {
            p{-1.0f, 1}, p{1.0f, 2}, p{1.0f, 3}, p{0.2f, 4}, p{-0.2f, 5}, p{3.0f, 6}, p{-1.0f, 7}, p{0.9f, 8}, p{0.1f, 9}, p{-0.1f, 10}
        }
    }
};

PlotPatternState<2> t00p00(PlotPattern<2>("target-00", { TEST_METRICS["target-00"][0], TEST_METRICS["target-00"][1] }));
PlotPatternState<2> t00p01(PlotPattern<2>("target-00", { TEST_METRICS["target-00"][2], TEST_METRICS["target-00"][3] }));
PlotPatternState<2> t00p02(PlotPattern<2>("target-00", { TEST_METRICS["target-00"][4], TEST_METRICS["target-00"][5] }));
PlotPatternState<2> t00p03(PlotPattern<2>("target-00", { TEST_METRICS["target-00"][6], TEST_METRICS["target-00"][7] }));
PlotPatternState<2> t00p04(PlotPattern<2>("target-00", { TEST_METRICS["target-00"][8], TEST_METRICS["target-00"][9] }));
vector<PlotPatternState<2>> t00 {t00p00, t00p01, t00p02, t00p03, t00p04};

PlotPatternState<2> t01p00(PlotPattern<2>("target-01", { TEST_METRICS["target-01"][0], TEST_METRICS["target-01"][1] }));
PlotPatternState<2> t01p01(PlotPattern<2>("target-01", { TEST_METRICS["target-01"][2], TEST_METRICS["target-01"][3] }));
PlotPatternState<2> t01p02(PlotPattern<2>("target-01", { TEST_METRICS["target-01"][4], TEST_METRICS["target-01"][5] }));
PlotPatternState<2> t01p03(PlotPattern<2>("target-01", { TEST_METRICS["target-01"][6], TEST_METRICS["target-01"][7] }));
PlotPatternState<2> t01p04(PlotPattern<2>("target-01", { TEST_METRICS["target-01"][8], TEST_METRICS["target-01"][9] }));
vector<PlotPatternState<2>> t01 {t01p00, t01p01, t01p02, t01p03, t01p04};

PlotPatternState<2> t02p00(PlotPattern<2>("target-02", { TEST_METRICS["target-02"][0], TEST_METRICS["target-02"][1] }));
PlotPatternState<2> t02p01(PlotPattern<2>("target-02", { TEST_METRICS["target-02"][2], TEST_METRICS["target-02"][3] }));
PlotPatternState<2> t02p02(PlotPattern<2>("target-02", { TEST_METRICS["target-02"][4], TEST_METRICS["target-02"][5] }));
PlotPatternState<2> t02p03(PlotPattern<2>("target-02", { TEST_METRICS["target-02"][6], TEST_METRICS["target-02"][7] }));
PlotPatternState<2> t02p04(PlotPattern<2>("target-02", { TEST_METRICS["target-02"][8], TEST_METRICS["target-02"][9] }));
vector<PlotPatternState<2>> t02 {t02p00, t02p01, t02p02, t02p03, t02p04};

auto goalPatternState = t00p01;

using STATE = PlotPatternState<2>;
using ACTION = PlotPatternState<2>;

SCENARIO("First order complexity test. Give a set of pattern P that entails a goal pattern g.") {
  GIVEN("A analytic-engine-environment is created.") {
    AnalyticEngineEnvironment<STATE, ACTION> aee;

    rl::ActuatorBase<STATE, ACTION> actuator(aee);
    for (auto p : t00) { actuator.addAction(p); }
    for (auto p : t01) { actuator.addAction(p); }
    for (auto p : t02) { actuator.addAction(p); }

    rl::Algorithm::Policy::EpsilonGreedy<STATE, ACTION> policy(1.0F);
    rl::Algorithm::RL::Sarsa<STATE, ACTION> sarsaAlgorithm(0.1F, 0.9F, policy);
    sarsaAlgorithm.setDefaultStateActionValue(-1000000.0F);

    rl::AgentSupervised<STATE, ACTION> agent(actuator, sarsaAlgorithm);

    WHEN ("When trained for 1 data points, we should have a solution.") {
      agent.train(
          t01p00,  // The initial state.
          // Note that action=nextState. The important thing is, if the action/nextState don't entail goalState, there will be negative reward.
          goalPatternState,
          -goalPatternState.getValue().getAbsoluteArea(goalPatternState.getValue()),  // Reward.
          goalPatternState  // We actually transition to the goal state.
      );
      agent.train(
          t01p01,
          t00p02,
          -goalPatternState.getValue().getAbsoluteArea(t00p02.getValue()),
          t00p02
      );
      agent.train(
          t01p02,
          t00p03,
          -goalPatternState.getValue().getAbsoluteArea(t00p03.getValue()),
          t00p03
      );

      THEN("State-Action value of t01p00->goal is the biggest.") {
        rl::FLOAT t01p00Value = sarsaAlgorithm.getStateActionValue(rl::StateAction<STATE, ACTION>(t01p00, goalPatternState));
        rl::FLOAT t01p01Value = sarsaAlgorithm.getStateActionValue(rl::StateAction<STATE, ACTION>(t01p01, goalPatternState));
        rl::FLOAT t01p02Value = sarsaAlgorithm.getStateActionValue(rl::StateAction<STATE, ACTION>(t01p02, goalPatternState));
        REQUIRE(t01p00Value > t01p01Value);
        REQUIRE(t01p00Value > t01p02Value);
      }
    }
  }
}