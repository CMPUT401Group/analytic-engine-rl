//
// Created by jandres on 07/11/16.
//

#pragma once

#include <vector>
#include <array>
#include <cassert>
#include <cmath>
#include <string>

#include "json.hpp"

#include "declares.h"
#include "spline.h"

using std::vector;
using std::array;
using std::string;
using json = nlohmann::json;

class Metric {
 public:
  using DATA = vector<ANALYTIC_ENGINE::point>;
  Metric(json& j) throw(std::domain_error) {
    if (j["target"].is_null()) {
      throw new std::domain_error("Given json is invalid.");
    }

    this->_metricName = j["target"];
    for (auto d : j["datapoints"]) {
      d[0] = d[0].is_null() ? 0 : (int)d[0];
      this->_data.push_back(ANALYTIC_ENGINE::point({d[0], d[1]}));
    }
  }

  bool operator>(const Metric& rhs) const {
    return this->getMetricName() > rhs.getMetricName();
  }

  bool operator<(const Metric& rhs) const {
    return this->getMetricName() < rhs.getMetricName();
  }

  bool operator>=(const Metric& rhs) const {
    return this->getMetricName() >= rhs.getMetricName();
  }

  bool operator<=(const Metric& rhs) const {
    return this->getMetricName() <= rhs.getMetricName();
  }

  bool operator==(const Metric& rhs) const {
    return this == &rhs || this->getMetricName() == rhs.getMetricName();
  }

  Metric(string metricName, const DATA& data) : _metricName(metricName), _data(data) {}

  ANALYTIC_ENGINE::time getDuration() const {
    return this->getTimeEnd() - this->getTimeBegin();
  }

  ANALYTIC_ENGINE::time getTimeBegin() const {
    return std::get<1>(this->_data.front());
  }

  ANALYTIC_ENGINE::time getTimeEnd() const {
    return std::get<1>(this->_data.back());
  }

  ANALYTIC_ENGINE::time getTimeAfter(ANALYTIC_ENGINE::time time) const {
    for (auto d : this->_data) {
      if (std::get<1>(d) >= time) {
        return std::get<1>(d);
      }
    }

    assert(false /* Given time exceeded range. */);
  }

  ANALYTIC_ENGINE::time getTimeBefore(ANALYTIC_ENGINE::time time) const {
    for (auto iter = this->_data.rbegin(); iter != this->_data.rend(); iter++) {
      auto d = *iter;
      if (std::get<1>(d) <= time) {
        return std::get<1>(d);
      }
    }

    assert(false /* Given time is less than range. */);
  }

  ANALYTIC_ENGINE::time getIndexAfter(ANALYTIC_ENGINE::time time) const {
    if (time > this->getTimeEnd()) {
      throw "time exceeded Metric::getTimeEnd()";
    }

    size_t index = 0;
    for (auto d : this->_data) {
      if (std::get<1>(d) >= time) {
        return index;
      }
      index++;
    }

    assert(false /* Given time exceeded range. */);
  }

  ANALYTIC_ENGINE::time getIndexBefore(ANALYTIC_ENGINE::time time) const {
    if (time < this->getTimeBegin()) {
      throw "time is less than Metric::getTimeBegin()";
    }

    size_t index = this->_data.size() - 1;
    for (auto iter = this->_data.rbegin(); iter != this->_data.rend(); iter++) {
      auto d = *iter;
      if (std::get<1>(d) <= time) {
        return index;
      }
      index--;
    }

    assert(false /* Given time is less than range. */);
  }

  DATA& getData() {
    return this->_data;
  }

  const DATA& getData() const {
    return this->_data;
  }

  string getMetricName() const {
    return this->_metricName;
  }

  template <size_t DURATION>
  PlotPattern<DURATION> getPattern(ANALYTIC_ENGINE::time tBegin, ANALYTIC_ENGINE::time tEnd) {
    assert(tEnd >= tBegin);

    ANALYTIC_ENGINE::time beginI = this->getIndexAfter(tBegin);
    ANALYTIC_ENGINE::time endI = this->getIndexBefore(tEnd);

    if (endI - beginI <= 2) {
      throw "Not enough resolution";
    }

    std::vector<double> x;
    std::vector<double> y;

    for (size_t i = beginI, j = 0; i < endI; i++, j++) {
      x.push_back(this->_data[i].second);
    }
    for (size_t i = beginI, j = 0; i < endI; i++, j++) {
      y.push_back(this->_data[i].first);
    }

    tk::spline interpolatedPattern;
    interpolatedPattern.set_points(x, y);

    typename PlotPattern<DURATION>::DATA data;
    double durationIncrement = static_cast<double>(tEnd - tBegin)/DURATION;
    for (size_t i = 0; i < DURATION; i++) {
      double time = static_cast<double>(tBegin) + durationIncrement*i;
      float y = interpolatedPattern(time);
      data[i] = std::pair<double, double>({y, time});
    }

    return PlotPattern<DURATION>(*this, data);
  }

  static std::vector<Metric*> parseMetrics(json metricsJSON) {
    std::vector<Metric*> metrics;
    for (auto metricJSON : metricsJSON) {
      // For some reason, exception is not being caught in try/catch block.
      if (metricJSON[0]["target"].is_null()) {
        //std::cerr << "Invalid metric" << std::endl;
        continue;
      }

      auto metric = new Metric(metricJSON[0]);
      metrics.push_back(metric);
    }

    return metrics;
  }

  static std::pair<ANALYTIC_ENGINE::time, ANALYTIC_ENGINE::time> getMinMaxTime(const vector<Metric*> metrics) {
    ANALYTIC_ENGINE::time minMetricTime = std::accumulate(
        metrics.begin(),
        metrics.end(),
        metrics[0]->getTimeBegin(),
        [](ANALYTIC_ENGINE::time currentMin, const Metric* m) {
          if (currentMin > m->getTimeBegin()) { return m->getTimeBegin(); }
          return currentMin;
        });

    // Get max metric time.
    ANALYTIC_ENGINE::time maxMetricTime = std::accumulate(
        metrics.begin(),
        metrics.end(),
        metrics[0]->getTimeBegin(),
        [](ANALYTIC_ENGINE::time currentMax, const Metric* m) {
          if (currentMax < m->getTimeEnd()) { return m->getTimeEnd(); }
          return currentMax;
        });

    return  {minMetricTime, maxMetricTime};
  }

  template<size_t PATTERN_SIZE>
  static vector<PlotPattern<PATTERN_SIZE>> getPatternsFromMetrics(
      const vector<Metric*> metrics,
      ANALYTIC_ENGINE::time patternTimeBegin,
      ANALYTIC_ENGINE::time patternTimeEnd) {
    vector<PlotPattern<PATTERN_SIZE>> patterns;
    for (auto metric : metrics) {
      try {
        patterns.push_back(metric->getPattern<PATTERN_SIZE>(patternTimeBegin, patternTimeEnd));
      }catch(exception& e) {
        std::cerr << e.what() << std::endl;
      }catch(...) {
        // Invalid resolution of metric.
      }
    }

    return patterns;
  }

 protected:
  DATA _data;
  string _metricName;
};

std::ostream& operator <<(std::ostream& stream, const Metric& pp) {
  stream << "{ name: " << pp.getMetricName() << ", data: { ";
  for (auto d : pp.getData()) {
    stream << "(" << std::get<0>(d) << ", " << std::get<1>(d) << "), ";
  }
  stream << " } }";
  return stream;
}