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
#include "../lib/spline.h"

using std::vector;
using std::array;
using std::string;

class Metric;

/*! \class PlotPattern
 *  \brief A pottern that is found inside Metric.
 *  \tparam RESOLUTION The number of times how x (time) is divided.
 */
template <size_t RESOLUTION>
class PlotPattern {
 public:
  using DATA = array<std::pair<double, double>, RESOLUTION>;

  /**
   * TODO(jandres): make equalityEpsilon a cli param.
   * TODO(jandres): Let this constructor extract those pattern, just give the tBegin and tEnd.
   *
   * @param metric Metric this pattern is related from.
   * @param data The data acquired from the emtric.
   * @param equalityEpsilon The lower the value, the lower the chance this pattern equals to other pattern
   *                        (frame same metric).
   */
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
        [](float currentMin, const app::point& p) {
          if (currentMin > p.first) { return p.first; }
          return currentMin;
        });

    this->_max = std::accumulate(
        data.begin(),
        data.end(),
        data.front().first,
        [](float currentMax, const app::point& p) {
          if (currentMax < p.first) { return p.first; }
          return currentMax;
        });
  }

  /**
   * @return The duration of this pattern.
   */
  double getDuration() const {
    return this->getTimeEnd() - this->getTimeBegin();
  }

  /**
   * @return The begin time of the pattern.
   */
  double getTimeBegin() const {
    return std::get<1>(this->_data.front());
  }

  /**
   * @return The end time of the pattern.
   */
  double getTimeEnd() const {
    return std::get<1>(this->_data.back());
  }

  bool operator<(const PlotPattern<RESOLUTION>& rhs) const {
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

  bool operator>(const PlotPattern<RESOLUTION>& rhs) const {
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

  bool operator<=(const PlotPattern<RESOLUTION>& rhs) const {
    return !(*this > rhs);
  }

  bool operator>=(const PlotPattern<RESOLUTION>& rhs) const {
    return !(*this < rhs);
  }

  bool operator==(const PlotPattern<RESOLUTION>& rhs) const {
    if (this == &rhs) {
      return true;
    }

    return *this->_metric == *rhs._metric &&
        this->getAbsoluteArea(rhs) < this->_equalityEpsilon;
  }

  bool operator!=(const PlotPattern<RESOLUTION>& rhs) const {
    return !(*this == rhs);
  }

  /**
   * Gets the absolute area between two pattern, ignoring intersection.
   * @param rhs The metric to calculate the diff from.
   * @return The accumulative diff all positive. Better for diff not being offseted by negative.
   */
  float getAbsoluteArea(const PlotPattern<RESOLUTION>& rhs) const {
    float area = 0.0;

    for (size_t i = 0; i < RESOLUTION; i++) {
      area += std::abs(this->getNormalizeY(i) - rhs.getNormalizeY(i));
    }

    return area/RESOLUTION;
  }

  /**
   * Gets the area between two pattern, taking into account the intersection.
   * @param rhs The metric to calculate the diff from.
   * @return The accumulative diff, including negative.
   */
  float getArea(const PlotPattern<RESOLUTION>& rhs) const {
    float area = 0.0;

    for (size_t i = 0; i < RESOLUTION; i++) {
      area += (this->getNormalizeY(i) - rhs.getNormalizeY(i));
    }

    return area/RESOLUTION;
  }

  /**
   * Given an index, acquires a normalize y value [0, 1].
   * @param index The index to acquire the y value.
   * @return A normalize y value [0, 1]
   */
  float getNormalizeY(size_t index) const {
    float magnitude = this->_max - this->_min;
    if (magnitude < 0.00000001f) {
      return 0.0f;
    }
    return (std::get<0>(this->_data.at(index)) - this->_min) / magnitude;
  }

  /**
   * @return The data associated with this metrics.
   */
  DATA& getData() {
    return this->_data;
  }

  /**
   * @return The data associated with this metrics.
   */
  const DATA& getData() const {
    return this->_data;
  }

  /**
   * @return The associated metric.
   */
  std::shared_ptr<Metric>& getMetric() {
    return this->_metric;
  }

  /**
   * @return The associated metric.
   */
  const std::shared_ptr<Metric>& getMetric() const {
    return this->_metric;
  }

  /**
   * @return The name of the associated metric.
   */
  string getMetricName() const {
    return this->_metric->getMetricName();
  }

  /**
   * @return Give a unique id for this pattern.
   */
  size_t getID() const {
    size_t id = 0;
    auto base = RESOLUTION * 10;
    for (size_t i = 0; i < RESOLUTION; i++) {
      id += std::ceil(this->getNormalizeY(i) * 5) * base * i;
    }

    return id;
  }

  virtual PlotPattern<RESOLUTION>& operator=(PlotPattern<RESOLUTION>& rhs) {
    if (this == &rhs) {
      return *this;
    }

    this->_data = rhs._data;
    this->_equalityEpsilon = rhs._equalityEpsilon;
    this->_metric = rhs._metric;
    return *this;
  }

  /**
   * Given a metricName return the index of the pattern with the associated metricName.
   * @param patterns A vector of patterns.
   * @param metricName The name of the metric.
   * @param patternIndex Index of the pattern with the associated pattern name.
   */
  static void getPatternIndexFromMetricName(vector<rl::spState<PlotPattern<RESOLUTION>>>& patterns,
                                     string metricName,
                                     size_t& patternIndex) {
    for (; patternIndex < patterns.size(); patternIndex++) {
      if (patterns[patternIndex]->getMetricName() == metricName) {
        break;
      }
    }
  }

  rl::spFloatVector getGradientDescentParameters() {
    rl::spFloatVector rv(new rl::floatVector());

    for (size_t i = 0; i < RESOLUTION; i++) {
      rv->push_back(this->getNormalizeY(i));
    }

    rv->push_back(static_cast<float>(this->_metric->getMetricIndex()));
    return rv;
  }

 protected:
  DATA _data;
  float _equalityEpsilon;
  std::shared_ptr<Metric> _metric;
  float _min, _max;
};

template <size_t RESOLUTION>
std::ostream& operator <<(std::ostream& stream, const PlotPattern<RESOLUTION>& pp) {
  stream << "{ name: " << pp.getMetricName() << ", data: { ";
  for (auto d : pp.getData()) {
    stream << "(" << std::get<0>(d) << ", " << std::get<1>(d) << "), ";
  }
  stream << " } }";
  return stream;
}

using PlotPatternSpecialized = PlotPattern<app::PATTERN_SIZE>;
using STATE = PlotPattern<app::PATTERN_SIZE>;
using ACTION = STATE;