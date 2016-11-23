//
// Created by jandres on 07/11/16.
//

#pragma once

#include <vector>
#include <array>
#include <cassert>
#include <cmath>
#include <string>
#include <functional>
#include <numeric>
#include <iostream>
#include <rl>
#include <memory>

#include "declares.h"
#include "spline.h"

using std::vector;
using std::array;
using std::string;

class Metric;

template <size_t DURATION>
class PlotPattern {
 public:
  using DATA = array<std::pair<double, double>, DURATION>;

  // todo: make equalityEpsilon a cli param.
  PlotPattern(const std::shared_ptr<Metric>& metric,
              const DATA& data,
              float equalityEpsilon = 0.08f) :
      _metric(metric),
      _data(data),
      _equalityEpsilon(equalityEpsilon) {
    this->_min = std::accumulate(
        data.begin(),
        data.end(),
        data.front().first,
        [](float currentMin, const ANALYTIC_ENGINE::point& p) {
          if (currentMin > p.first) { return p.first; }
          return currentMin;
        });

    this->_max = std::accumulate(
        data.begin(),
        data.end(),
        data.front().first,
        [](float currentMax, const ANALYTIC_ENGINE::point& p) {
          if (currentMax < p.first) { return p.first; }
          return currentMax;
        });
  }

  double getDuration() const {
    return DURATION;
  }

  double getTimeBegin() const {
    return std::get<1>(this->_data.front());
  }

  double getTimeEnd() const {
    return std::get<1>(this->_data.back());
  }

  bool operator<(const PlotPattern<DURATION>& rhs) const {
    if (*(this->_metric) < *(rhs._metric)) {
      return true;
    }
    if (*(this->_metric) > *(rhs._metric)) {
      return false;
    }
    if (this->getID() < rhs.getID()) {
      return true;
    }
    return false;
  }

  bool operator>(const PlotPattern<DURATION>& rhs) const {
    if (*(this->_metric) > *(rhs._metric)) {
      return true;
    }
    if (*(this->_metric) < *(rhs._metric)) {
      return false;
    }
    if (this->getID() > rhs.getID()) {
      return true;
    }
    return false;
  }

  bool operator<=(const PlotPattern<DURATION>& rhs) const {
    return !(*this > rhs);
  }

  bool operator>=(const PlotPattern<DURATION>& rhs) const {
    return !(*this < rhs);
  }

  bool operator==(const PlotPattern<DURATION>& rhs) const {
    if (this == &rhs) {
      return true;
    }

    return *this->_metric == *rhs._metric &&
        this->getAbsoluteArea(rhs) < this->_equalityEpsilon;
  }

  bool operator!=(const PlotPattern<DURATION>& rhs) const {
    return !(*this == rhs);
  }

  float getAbsoluteArea(const PlotPattern<DURATION>& rhs) const {
    float area = 0.0;

    for (size_t i = 0; i < DURATION; i++) {
      area += std::abs(this->getNormalizeY(i) - rhs.getNormalizeY(i));
    }

    return area/DURATION;
  }

  float getArea(const PlotPattern<DURATION>& rhs) const {
    float area = 0.0;

    for (size_t i = 0; i < DURATION; i++) {
      area += (this->getNormalizeY(i) - rhs.getNormalizeY(i));
    }

    return area/DURATION;
  }

  float getNormalizeY(size_t index) const {
    float magnitude = this->_max - this->_min;
    return (std::get<0>(this->_data.at(index)) - this->_min) / magnitude;
  }

  DATA& getData() {
    return this->_data;
  }

  const DATA& getData() const {
    return this->_data;
  }

  std::shared_ptr<Metric>& getMetric() {
    return this->_metric;
  }

  const std::shared_ptr<Metric>& getMetric() const {
    return this->_metric;
  }

  string getMetricName() const {
    return this->_metric->getMetricName();
  }

  size_t getID() const {
    size_t id = 0;
    auto base = DURATION * 10;
    for (size_t i = 0; i < DURATION; i++) {
      id += std::ceil(this->getNormalizeY(i) * 5) * base * i;
    }

    return id;
  }

  virtual PlotPattern<DURATION>& operator=(PlotPattern<DURATION>& rhs) {
    if (this == &rhs) {
      return *this;
    }

    this->_data = rhs._data;
    this->_equalityEpsilon = rhs._equalityEpsilon;
    this->_metric = rhs._metric;
    return *this;
  }

  static void getPatternIndexFromMetricName(vector<rl::spState<PlotPattern<DURATION>>>& patterns,
                                     string metricName,
                                     size_t& patternIndex) {
    for (; patternIndex < patterns.size(); patternIndex++) {
      if (patterns[patternIndex]->getMetricName() == metricName) {
        break;
      }
    }
  }

 protected:
  DATA _data;
  float _equalityEpsilon;
  std::shared_ptr<Metric> _metric;

  float _min, _max;
};

template <size_t DURATION>
std::ostream& operator <<(std::ostream& stream, const PlotPattern<DURATION>& pp) {
  stream << "{ name: " << pp.getMetricName() << ", data: { ";
  for (auto d : pp.getData()) {
    stream << "(" << std::get<0>(d) << ", " << std::get<1>(d) << "), ";
  }
  stream << " } }";
  return stream;
}

using PlotPatternSpecialized = PlotPattern<ANALYTIC_ENGINE::PATTERN_SIZE>;