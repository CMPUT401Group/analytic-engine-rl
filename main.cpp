#include <iostream>
#include <rl/Sarsa.h>  // librl
#include <array>

#include "plot-pattern-state.h"

using namespace std;

int main(int argc, char** argv) {
  PlotPatternState<2> plotPatternState1(PlotPattern<2>("metric01", { std::make_pair(400, 0.0f), std::make_pair(300, 1.0f) }));
  PlotPatternState<2> plotPatternState2(PlotPattern<2>("metric02", { std::make_pair(401, 0.0f), std::make_pair(302, 1.0f) }));
  PlotPatternState<2> plotPatternState3(PlotPattern<2>("metric02", { std::make_pair(401, 1.0f), std::make_pair(302, 1.0f) }));
  std::cout << (plotPatternState1 == plotPatternState2) << std::endl;
  std::cout << (plotPatternState1 < plotPatternState2) << std::endl;
  std::cout << (plotPatternState1 > plotPatternState2) << std::endl;
  std::cout << (plotPatternState1 >= plotPatternState2) << std::endl;
  std::cout << (plotPatternState1 <= plotPatternState2) << std::endl;
  std::cout << (plotPatternState2 == plotPatternState3) << std::endl;
  std::cout << "Hello World" << std::endl;
  return 0;
}