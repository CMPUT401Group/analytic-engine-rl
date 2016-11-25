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

# Reinforcement Learning and Metric Analysis
This just seems like a perfect combination. The basic concept is **P -> g**'s is given
a positive reinforcement when it occurs, but negative reinforcement if **P -> (not g)**
occurs. The latter eliminates false positives, for instance if **p->q** and not **q->p**, 
**q->p** (false positive) will be eliminated. We take _n_ random samples around the metrics (known as training). 
As _n_ increase and the bigger our metric time span, the better AI's prediction model is 
(less false positives and such), much like a human observer. 

# Compilation and Installation

### Dependency:
* g++-4.9 or greater
* armadillo
* cppunit
* [JoeyAndres/rl v2.0.1-beta.3](https://github.com/JoeyAndres/rl/releases/tag/v2.0.1-beta.3) hehe, my pride and joy.

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

1. `git clone https://github.com/CMPUT401Group/analytic-engine-rl.git`
2. cd analytic-engine-rl
3. `mkdir build`
4. `cd build`
5. `cmake ..`
6. `make -j16`

# Usage
While still in the build directory:

```bash
# This will take a while to run due to 11,000 metrics to process.
./analytic-engine-rl-cli test/data/test-metrics.json test/data/config.json
```

Parameters:
* **test/data/test-metrics.json**: This is simply a list of metrics. The format is:
```js
[
  {"target": "metricName", datapoints: [ [y, x], ...]},
  ...
]
```
* **test/data/config.js**: Tells the cli program what is the goal pattern and some
reinforcement learning parameters that should be good enough.
```js
{
  // The goal pattern to which our AI will try to find entailing patterns from
  // other metric.
  "goalPattern": {
    // Name of the metric in which our pattern is located.
    "metric": "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
    
    // The begin time of our goal pattern (unix time stamp) in the metric.
    "timeBegin": 1474111311,
    
    // The end time of our goal pattern (unix time stamp) in the metric.  
    "timeEnd": 1474111440
  },
  
  // The number of times our AI sample the metrics. The higher the more accurate the
  // model, but slower.
  "iterationCount": 50,
  
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
  {
      "destPattern": {
        "metric": "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
        "timeBegin": 1474111311,
        "timeEnd": 1474111311
      },
      "reward": -1000000,
      "sourcePattern": {
        "metric": "invidi.webapp.localhost_localdomain.database.request.findEtl.success_gauge",
        "timeBegin": 1474111311,
        "timeEnd": 1474111311
      }
  },
// The last few outputs.
// (Note: this is ran with scanStepSize=40, so results are inaccurate but should suffice).
  {
    "destPattern": {
      "metric": "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
      "timeBegin": 1474111311,
      "timeEnd": 1474111311
    },
    "reward": -979290.007555472,
    "sourcePattern": {
      "metric": "invidi.webapp.localhost_localdomain.database.request.findAdsToKeep.start_gauge",
      "timeBegin": 1474111311,
      "timeEnd": 1474111311
    }
  },
  {
    "destPattern": {
      "metric": "invidi.webapp.localhost_localdomain.request.total_response_time.mean",
      "timeBegin": 1474111311,
      "timeEnd": 1474111311
    },
    "reward": -979290.007555472,
    "sourcePattern": {
      "metric": "invidi.webapp.localhost_localdomain.cache.DEVICE_ID.count",
      "timeBegin": 1474111311,
      "timeEnd": 1474111311
    }
  }
]
```

* **sourcePattern**: The entailing pattern.
  * **metric:** The metric in which this pattern is located.
  * **timeBegin:** The beginning time where this pattern is located in the metric.
  * **timeEnd:** The end time where this pattern is located in the metric.
* **destPattern:**  The entailed pattern.
  * **metric:** The metric in which this pattern is located.
  * **timeBegin:** The beginning time where this pattern is located in the metric.
  * **timeEnd:** The end time where this pattern is located in the metric.
* **reward:** The value of transitioning from **sourcePattern** to **destPattern**. Due
to the nature of this problem this values range from _-infinity_ to 0.

Since we are still in the incipient stage of this project, **timeBegin**/**timeEnd** for
both **sourcePattern** and **destPattern** is the same. This appears to be giving
useful results, but there is nothing stopping us from having those having some offsets
other than it would slow the development due to more things to test.

# TODOs
* Not restrict to one level of entailment.
* Not restrict source/destination Pattern to single timeBegin/timeEnd.
* At the moment, we only use SARSA algorithm. There are actually better algorithms that allows 
for faster convergence, but we are still exploring so I'll stick with the simplest.
**Allow to specify algorithm in config.json.**
* [Reinforcment Learning Module](https://github.com/JoeyAndres/rl)
  * Allow multi-threading. Atm, I have only explored cpu intrinsics (SSE/SSE2/AVX and such),
  but we could really benefit from multi-threading.
  * Atm, the AI model is stored in RAM. We should be able to store results in JSON file, or
  even better, utilized MongoDB (will help with many node js development).
* Visualization. Imagine multi-level entailment and visualization. That would be REALLY
sweet. That is why I mentioned storing MongoDB as one of TODO, since with MeteorJS for
node development, we could have our visualization updated automatically in real-time as
our model changes in the backend.