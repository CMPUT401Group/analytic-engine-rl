//
// Created by jandres on 07/11/16.
//

#pragma once

#include <vector>
#include <array>
#include <cassert>
#include <cmath>
#include <string>

#include "declares.h"

using std::vector;
using std::array;
using std::string;

template <size_t DURATION>
class PlotPattern {
 public:
  using DATA = array<ANALYTIC_ENGINE::point, DURATION>;
  PlotPattern(string metricName, const DATA& data, float equalityEpsilon = 0.001f) :
      _metricName(metricName),
      _data(data),
      _equalityEpsilon(equalityEpsilon) {}

  ANALYTIC_ENGINE::time getDuration() const {
    return DURATION;
  }

  ANALYTIC_ENGINE::time getTimeBegin() const {
    return std::get<1>(this->_data.begin());
  }

  ANALYTIC_ENGINE::time getTimeEnd() const {
    return std::get<1>(this->_data.end());
  }

  bool operator<(const PlotPattern<DURATION>& rhs) const {
    return *this != rhs &&
        (this->_metricName < rhs._metricName ||
            (this->_metricName == rhs._metricName && this->getArea(rhs) < 0));
  }

  bool operator>(const PlotPattern<DURATION>& rhs) const {
    return *this != rhs &&
        (this->_metricName > rhs._metricName ||
            (this->_metricName == rhs._metricName && this->getArea(rhs) > 0));
  }

  bool operator<=(const PlotPattern<DURATION>& rhs) const {
    return *this == rhs &&
        (this->_metricName <= rhs._metricName ||
            (this->_metricName == rhs._metricName && this->getArea(rhs) < 0));
  }

  bool operator>=(const PlotPattern<DURATION>& rhs) const {
    return *this == rhs &&
        (this->_metricName >= rhs._metricName ||
            (this->_metricName == rhs._metricName && this->getArea(rhs) > 0));
  }

  bool operator==(const PlotPattern<DURATION>& rhs) const {
    if (this == &rhs) {
      return true;
    }

    return this->_metricName == rhs._metricName &&
        this->getAbsoluteArea(rhs) < this->_equalityEpsilon;
  }

  bool operator!=(const PlotPattern<DURATION>& rhs) const {
    return !(*this == rhs);
  }

  float getAbsoluteArea(const PlotPattern<DURATION>& rhs) const {
    float area = 0.0;

    for (size_t i = 0; i < DURATION; i++) {
      area += std::abs(std::get<0>(rhs._data.at(i)) - std::get<0>(this->_data.at(i)));
    }

    return area;
  }

  float getArea(const PlotPattern<DURATION>& rhs) const {
    float area = 0.0;

    for (size_t i = 0; i < DURATION; i++) {
      area += std::get<0>(rhs._data.at(i)) - std::get<0>(this->_data.at(i));
    }

    return area;
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

  virtual PlotPattern<DURATION>& operator=(PlotPattern<DURATION>& rhs) {
    if (this == &rhs) {
      return *this;
    }

    this->_data = rhs._data;
    this->_equalityEpsilon = rhs._equalityEpsilon;
    this->_metricName = rhs._metricName;
    return *this;
  }

 protected:
  DATA _data = nullptr;
  float _equalityEpsilon;
  string _metricName;
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