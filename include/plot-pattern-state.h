//
// Created by jandres on 07/11/16.
//

#pragma once

#include <iostream>
#include <rl>
#include <vector>

#include "declares.h"
#include "plot-pattern.h"

using namespace std;

template <size_t DURATION>
class PlotPatternState : public rl::StateInterface<PlotPattern<DURATION>> {
 public:
  // TODO: Remove getValue thing. Simply have a state interface and let it be that.
  PlotPatternState(const rl::spState<PlotPattern<DURATION>>& value) : rl::StateInterface<PlotPattern<DURATION>>(value) {}

  static vector<PlotPatternState<DURATION>> getPlotPatternStatesFromPatterns(
      const vector<rl::spState<PlotPattern<DURATION>>>& patterns) {
    vector<rl::spState<PlotPatternState<DURATION>>> patternStates;
    for (auto pattern : patterns) {
      patternStates.push_back(PlotPatternState<DURATION>(pattern));
    }

    return patternStates;
  }
};

template <size_t DURATION>
std::ostream& operator <<(std::ostream& stream, const PlotPatternState<DURATION>& pps) {
  stream << *(pps.getValue());
  return stream;
}

using STATE = PlotPattern<ANALYTIC_ENGINE::PATTERN_SIZE>;
using ACTION = STATE;