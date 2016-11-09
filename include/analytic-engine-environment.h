//
// Created by jandres on 07/11/16.
//

#pragma once

#include <map>

#include <rl/GlobalHeader.h>  // types.
#include <rl/StateAction.h>
#include <rl/Environment.h>

#include "declares.h"
#include "plot-pattern-state.h"

using std::map;

template<size_t PATTERN_DURATION, size_t METRIC_DURATION>
class AnalyticEngineEnvironment : public AI::Environment<PlotPatternState<PATTERN_DURATION>, PlotPatternState<PATTERN_DURATION>> {
  using STATE = PlotPatternState<PATTERN_DURATION>;
  using ACTION = STATE;
 public:
  AnalyticEngineEnvironment(const map<string,  array<ANALYTIC_ENGINE::point, METRIC_DURATION>>& metrics) {

  }

  // Overloaded methods.
  virtual const STATE& getLastObservedState() const {

  }

  virtual AI::FLOAT getLastObservedReward() const {

  }

  virtual AI::FLOAT applyAction(const ACTION& Action) {

  }

  virtual void reset() {
    this->_pCurrentState = nullptr;
  }

 private:
  STATE* _pCurrentState = nullptr;
};