#pragma once

#include <rl/SensorDiscrete.h>

#include "plot-pattern.h"

template<size_t PATTERN_DURATION>
class AnalyticEngineSensor :
    public AI::SensorDiscrete
        <PlotPatternState<PATTERN_DURATION>,
         PlotPatternState<PATTERN_DURATION>> {
 public:
  using ENVIRONMENT = AI::Environment<PlotPatternState<PATTERN_DURATION>, PlotPatternState<PATTERN_DURATION>>;
  AnalyticEngineSensor(ENVIRONMENT& env) :
      AI::SensorDiscrete
          <PlotPatternState<PATTERN_DURATION>,
           PlotPatternState<PATTERN_DURATION>>(env){}
};