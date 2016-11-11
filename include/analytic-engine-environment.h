//
// Created by jandres on 07/11/16.
//

#pragma once

#include <map>
#include <rl>

#include "declares.h"
#include "plot-pattern-state.h"

using std::map;

template<class STATE, class ACTION>
class AnalyticEngineEnvironment : public AI::Environment<STATE, ACTION> {
 public:
  AnalyticEngineEnvironment() {}

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