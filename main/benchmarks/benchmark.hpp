#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <vector>
#include <chrono>
#include "dfl/dfl.hpp"
#include <numeric>

using namespace dfl;

template <class Tp>
inline void DoNotOptimize(Tp& value) {
    asm volatile("" : "+r,m"(value) : : "memory");
}

#define quote(a) #a

#define BENCHMARK(NAME, RUNTIME_MAX, FUNC) { benchmark x(NAME, RUNTIME_MAX, FUNC); x.run(); }

template<typename F>
class benchmark {
 public:
  benchmark(const char* name, std::chrono::seconds duration, F f) : _name{name}, _duration{duration}, _func{f} {}

  void run(){
    using clock = std::chrono::high_resolution_clock;
    std::vector<int32_t> measurements;
    auto during = clock::now();
    size_t iters = 0;
    auto runtime = std::chrono::nanoseconds(0);
    auto init = clock::now();
    while((during-init) < _duration){
      iters++;
      vTaskDelay(1);
      auto start = clock::now();
      _func();
      during = clock::now();
      runtime += during-start;
      auto t_dur = std::chrono::duration_cast<std::chrono::microseconds>(during - start).count();
      measurements.push_back(t_dur);
    }
    float duration = std::accumulate(measurements.begin(), measurements.end(), 0.0);
    float average = (duration / measurements.size());
    int64_t min = *std::min_element(measurements.begin(), measurements.end());
    int64_t max = *std::max_element(measurements.begin(), measurements.end());
    
    printf("[%-35.35s]: avg: %.2f\t+-%4.2f\titers: %d\n"
      , _name
      , average/1000
      , std::max(std::abs(average-min) , std::abs(average-max))/1000
      , iters
    );
  }

 private:
  const char* _name;
  std::chrono::seconds _duration;
  F _func{};
};
