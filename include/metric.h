//
// Created by jandres on 07/11/16.
//

#pragma once

#include <vector>
#include <array>
#include <cassert>
#include <cmath>
#include <string>
#include <memory>

#include "../lib/json.hpp"

#include "declares.h"
#include "../lib/spline.h"

using std::vector;
using std::array;
using std::string;
using json = nlohmann::json;

/*! \class Metric
 *  \brief Represents graphite metric.
 */
class Metric {
 public:
  // The data type of how the metric is stored.
  using DATA = vector<ANALYTIC_ENGINE::point>;

  /**
   * JSON constructor.
   * @param j Json metric (from graphite) to be parsed.
   */
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

  /**
   * Constructor.
   * @param metricName Name of the metric. 
   * @param data The data in the metric.
   */
  Metric(string metricName, const DATA& data) :
      _metricName(metricName),
      _data(data) {}

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

  /**
   * Getter for metric duration.
   * @return Duration of this metric (unixtimestamp).
   */
  ANALYTIC_ENGINE::time getDuration() const {
    return this->getTimeEnd() - this->getTimeBegin();
  }

  /**
   * Getter for earliest time for the metric.
   * @return The earliest time in metric (unixtimestamp). 
   */
  ANALYTIC_ENGINE::time getTimeBegin() const {
    return std::get<1>(this->_data.front());
  }

  /**
   * Getter for end time for the metric.
   * @return The end time in metric (unixtimestamp). 
   */
  ANALYTIC_ENGINE::time getTimeEnd() const {
    return std::get<1>(this->_data.back());
  }

  /**
   * Given a time, returns the nearest "greater time" located in the metric.
   * @param time (unixtimestamp).
   * @return the nearest "greater time" located in the metric.
   */
  ANALYTIC_ENGINE::time getTimeAfter(ANALYTIC_ENGINE::time time) const {
    for (auto d : this->_data) {
      if (std::get<1>(d) >= time) {
        return std::get<1>(d);
      }
    }

    assert(false /* Given time exceeded range. */);
  }

  /***
   * Given a time, returns the nearest "time less-than" located in the metric.
   * @param time (unixtimestamp).
   * @return the nearest "time less-than" located in the metric.
   */
  ANALYTIC_ENGINE::time getTimeBefore(ANALYTIC_ENGINE::time time) const {
    for (auto iter = this->_data.rbegin(); iter != this->_data.rend(); iter++) {
      auto d = *iter;
      if (std::get<1>(d) <= time) {
        return std::get<1>(d);
      }
    }

    assert(false /* Given time is less than range. */);
  }

 /**
   * Given a time, returns the nearest index greater than the current time.
   * @param time (unixtimestamp).
   * @return the nearest index greater than the current time index.
   */
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

  /**
   * Given a time, returns the nearest index less-than than the current time.
   * @param time (unixtimestamp).
   * @return the nearest index less than the current time.
   */
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

  /**
   * To retrieve the metrics.
   * @return The data represented by the metrics. 
   */
  DATA& getData() {
    return this->_data;
  }

  /**
   * To retrieve the metrics.
   * @return The data represented by the metrics. 
   */
  const DATA& getData() const {
    return this->_data;
  }

  /**
   * To retrieve the metric name.
   * @return The name of the metric.
   */
  string getMetricName() const {
    return this->_metricName;
  }

  /**
   * Acquires a pattern from a given matrix, given a tBegin, and tEnd.
   * @static
   * @tparam RESOLUTION Resolution of pattern to generate.
   *
   * @param metric Metric to extract pattern from.
   * @param tBegin The beginning time in metric.
   * @param tEnd The end time in metric.
   * @return extracted pattern.
   */
  template <size_t RESOLUTION>
  static rl::spState<PlotPattern<RESOLUTION>> getPattern(const std::shared_ptr<Metric>& metric,
                                                       ANALYTIC_ENGINE::time tBegin,
                                                       ANALYTIC_ENGINE::time tEnd) {
    assert(tEnd > tBegin);

    ANALYTIC_ENGINE::time beginI = metric->getIndexAfter(tBegin);
    ANALYTIC_ENGINE::time endI = metric->getIndexBefore(tEnd);

    if (endI - beginI <= 2) {
      throw "Not enough resolution";
    }

    std::vector<double> x;
    std::vector<double> y;

    for (size_t i = beginI, j = 0; i < endI; i++, j++) {
      x.push_back(metric->_data[i].second);
    }
    for (size_t i = beginI, j = 0; i < endI; i++, j++) {
      y.push_back(metric->_data[i].first);
    }

    tk::spline interpolatedPattern;
    interpolatedPattern.set_points(x, y);

    typename PlotPattern<RESOLUTION>::DATA data;
    double durationIncrement = static_cast<double>(tEnd - tBegin)/RESOLUTION;
    for (size_t i = 0; i < RESOLUTION; i++) {
      double time = static_cast<double>(tBegin) + durationIncrement*i;
      float y = interpolatedPattern(time);
      data[i] = std::pair<double, double>({y, time});
    }

    return rl::spState<PlotPattern<RESOLUTION>>(new PlotPattern<RESOLUTION>(metric, data));
  }

  /**
   * Given a json representing an array of metrics, returns an array of shared_ptr<Metric>.
   * @param metricsJSON The json representing an array of metrics.
   * @return an array of shared_ptr<Metric>.
   */
  static std::vector<std::shared_ptr<Metric>> parseMetrics(json metricsJSON) {
    std::vector<std::shared_ptr<Metric>> metrics;
    for (auto metricJSON : metricsJSON) {
      // For some reason, exception is not being caught in try/catch block.
      if (metricJSON["target"].is_null()) {
        //std::cerr << "Invalid metric" << std::endl;
        continue;
      }

      metrics.push_back(std::shared_ptr<Metric>(new Metric(metricJSON)));
    }

    return metrics;
  }

  /**
   * Given a vector of metrics, acquires the min/max time.
   * @param metrics Array of metrics to acquire the min/max time.
   * @return <min time, max time> both in unix timestamps.
   *
   */
  static std::pair<ANALYTIC_ENGINE::time, ANALYTIC_ENGINE::time> getMinMaxTime(const vector<shared_ptr<Metric>>& metrics) {
    ANALYTIC_ENGINE::time minMetricTime = std::accumulate(
        metrics.begin(),
        metrics.end(),
        metrics[0]->getTimeBegin(),
        [](ANALYTIC_ENGINE::time currentMin, const shared_ptr<Metric>& m) {
          if (currentMin > m->getTimeBegin()) { return m->getTimeBegin(); }
          return currentMin;
        });

    // Get max metric time.
    ANALYTIC_ENGINE::time maxMetricTime = std::accumulate(
        metrics.begin(),
        metrics.end(),
        metrics[0]->getTimeBegin(),
        [](ANALYTIC_ENGINE::time currentMax, const shared_ptr<Metric>& m) {
          if (currentMax < m->getTimeEnd()) { return m->getTimeEnd(); }
          return currentMax;
        });

    return  {minMetricTime, maxMetricTime};
  }

  /**
   * Acquires patterns from a given matrix, given a tBegin, and tEnd.
   * @static
   * @tparam PATTERN_SIZE The resolution of the x axist to be extracted. The higher, the higher the definition.
   * @param metrics An array of metric to extract a pattern from.
   * @param patternTimeBegin The begin time of the pattern to extract wihtin the metric.
   * @param patternTimeEnd The end time of the pattern to extract within the metric.
   * @return array of extracted pattern.
   */
  template<size_t PATTERN_SIZE>
  static vector<rl::spState<PlotPattern<PATTERN_SIZE>>> getPatternsFromMetrics(
      const vector<shared_ptr<Metric>> metrics,
      ANALYTIC_ENGINE::time patternTimeBegin,
      ANALYTIC_ENGINE::time patternTimeEnd) {
    vector<rl::spState<PlotPattern<PATTERN_SIZE>>> patterns;
    for (auto metric : metrics) {
      try {
        patterns.push_back(Metric::getPattern<PATTERN_SIZE>(
            metric,
            patternTimeBegin,
            patternTimeEnd));
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

inline std::ostream& operator <<(std::ostream& stream, const Metric& pp) {
  stream << "{ name: " << pp.getMetricName() << ", data: { ";
  for (auto d : pp.getData()) {
    stream << "(" << std::get<0>(d) << ", " << std::get<1>(d) << "), ";
  }
  stream << " } }";
  return stream;
}