//
// Created by jandres on 07/11/16.
//

#pragma once

#include <iostream>
#include <rl>
#include <vector>

#include "plot-pattern.h"

using namespace std;

template <size_t DURATION>
class PlotPatternState : public AI::StateInterface<PlotPattern<DURATION>> {
 public:
  PlotPatternState(const PlotPattern<DURATION>& value) : AI::StateInterface<PlotPattern<DURATION>>(value) {}

  static vector<PlotPatternState<DURATION>> getPlotPatternStatesFromPatterns(
      const vector<PlotPattern<DURATION>>& patterns) {
    vector<PlotPatternState<DURATION>> patternStates;
    for (auto pattern : patterns) {
      patternStates.push_back(PlotPatternState<DURATION>(pattern));
    }

    return patternStates;
  }
};

template <size_t DURATION>
std::ostream& operator <<(std::ostream& stream, const PlotPatternState<DURATION>& pps) {
  stream << pps.getValue();
  return stream;
}