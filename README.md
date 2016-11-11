analytic-engine-rl
==========

Analytic Engine RL (Reinforcement Learning) module. Given a _goal metric_ 
in some timespan (a _pattern_) **g**, this module will allow us to see what _pattern(s)_ **P** 
in other metrics will _Likely_ lead to **g**. A tool that can be very useful when analyzing
thousands of metrics.

**Note:**
> You might've noticed that given a goal pattern **g**, this module only gives a 
> set of pattern **P** that entails **g**. This is just one level entailment. The 
> method or the _RL Engine_ used can actually do multi-level entailment. But since
> this project is still in its incipient stage, we stick with one level.

# Compilation and Installation

### Dependency:
* g++-4.9 or greater
* armadillo
* cppunit
* [JoeyAndres/rl](https://github.com/JoeyAndres/rl) hehe, my pride and joy.

### Installing dependencies Ubuntu:

```bash
# Install everything that can be installed from ubuntu package manager.
sudo apt install libarmadillo-dev libcppunit-dev g++

# Install my pride and joy.
cd /tmp
git clone https://github.com/JoeyAndres/rl.git
cd rl
mkdir build
cd build
cmake ..
make -j16
make install
```

### Building
Once the dependencies are installed, you can start building:

1. `mkdir build`
2. `cd build`
3. `cmake ..`
4. `make -j16`

# Usage
While still in the build directory:

```bash
# This will take a while to run due to 11,000 metrics to process.
./analytic-engine-rl-cli test/data/test-metrics.json test/data/config.json
```

Parameters:
* **test/data/test-metrics.json**: This is simply a list of metrics. The format is:
```json
[
  {"target": "metricName", datapoints: [ [y, x], ...]},
  ...
]
```
* **test/data/config.js**: Tells the cli program what is the goal pattern and some
reinforcement learning parameters that should be good enough.
```js
{
  // The metric in our goal pattern. This metric should be in the first cli argument.
  "goalMetric": "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
  
  // The begin time of our goal pattern (unix time stamp).
  "goalPatternTimeBegin": 1474111311,
  
  // The end time of our goal pattern (unix time stamp).
  "goalPatternTimeEnd": 1474111440,
  
  // As we train our ai model, this is the increment in time in which we scan our metrics.
  // The smaller, the more accurate the results, but the slower. You can ramp this up to 
  // 40 just to get some results, though those won't be accurate.
  "scanStepSize": 2,
  
  // This the initial reward for all new states. Since all reward are negative
  // in this environment, you can afford to go as low as you want.
  "initialReward": -1000000.0,
  
  "reinforcementLearning": {
    // The higher, the quicker our ai model converge to a solution, but less accurate. 0.1 is fine.
    "stepSize": 0.1,
    
    // Discount rate. Not really useful atm since we only have one level of entailment. 
    // Once we multi-level of entailment is opened up, a low number (close to 0) means
    // entailed pattern multiple levels ahead will have almost no influence on current
    // pattern's rewRd. A high discountRate (close to 1) means entailed pattern multiple
    // levels ahead will have more significant influence in our current pattern's reward.
    "discountRate": 0.9
  },
  
  // The output file of our result.
  "resultFile": "result.json"
}
```
### Interpreting the result
In the result.json after running the the cli program with the test parameters should
output: 

```js
[
// First few outputs.
// ...
  [
    "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
    "invidi.webapp.localhost_localdomain.request.total_response_time.percentile.95",
    -1000000
  ],
  [
    "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
    "invidi.webapp.localhost_localdomain.request.total_response_time.percentile.98",
    -1000000
  ],
// The last few outputs.
// (Note: this is ran with scanStepSize=40, so results are inaccurate but should suffice).
  [
    "invidi.webapp.localhost_localdomain.database.request.findDevice.start_gauge",
    "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
    -965610.023177183
  ],
  [
    "invidi.webapp.localhost_localdomain.database.request.findAdsToKeep.success_gauge",
    "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
    -965610.023177183
  ],
  [
    "invidi.webapp.localhost_localdomain.database.request.findAdsToKeep.start_gauge",
    "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
    -965610.023177183
  ],
  [
    "invidi.webapp.localhost_localdomain.cache.DEVICE_ID.count",
    "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
    -965610.023177183
  ]
]
```

The output is a massive array. Each array entry contains _entailing pattern_,
_entailed pattern_, and _reward_. As expected, our goal pattern _invidi.webapp.localhost_localdomain.request.total_response_time.mean_
made up the _entailed patterns_ in the last few outputs with the greatest reward. In this
AI model, _invidi.webapp.localhost_localdomain.cache.DEVICE_ID.count_ is the greatest suspect
that entails our goal pattern (again, we are running with _scanStepSize=40_ so this is very inaccurate).

**Note:**
> that a pattern should also contain a _timeBegin_ and _timeEnd_, but since
> we are still in incipient stage, all patterns will be the same as config.json's
> "goalPatternTimeBegin" and "goalPatternTimeEnd".
