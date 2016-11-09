//
// Created by jandres on 07/11/16.
//

#pragma once

#include <rl/StateInterface.h>
#include <iostream>

#include "plot-pattern.h"

template <size_t DURATION>
class PlotPatternState : public AI::StateInterface<PlotPattern<DURATION>> {
 public:
  PlotPatternState(const PlotPattern<DURATION>& value) : AI::StateInterface<PlotPattern<DURATION>>(value) {}
};

template <size_t DURATION>
std::ostream& operator <<(std::ostream& stream, const PlotPatternState<DURATION>& pps) {
  stream << pps.getValue();
  return stream;
}