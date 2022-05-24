#include <stdio.h>
#include <stdlib.h>
#include <functional>
#include <vector>
#include <chrono>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "dfl/dfl.hpp"
#include "driver/adc.h"
#include <numeric>

using namespace dfl;

template <class Tp>
inline void DoNotOptimize(Tp& value) {
    asm volatile("" : "+r,m"(value) : : "memory");
}

template<typename F>
class benchmark {
 public:
  benchmark(const char* name, int32_t iters, F f) : _name{name}, _iters{iters}, _func{f} {}

  void run(){
    std::vector<int64_t> measurements;
    for (int32_t i = 0; i < _iters; ++i){
      vTaskDelay(1);
      auto start = std::chrono::high_resolution_clock::now();
      _func();
      auto during = std::chrono::high_resolution_clock::now();
      auto t_dur = std::chrono::duration_cast<std::chrono::microseconds>(during - start).count();
      measurements.push_back(t_dur);
    }
    float duration = std::accumulate(measurements.begin(), measurements.end(), 0.0);
    float average = (duration / measurements.size());
    int32_t min = *std::min_element(measurements.begin(), measurements.end());
    int32_t max = *std::max_element(measurements.begin(), measurements.end());
    
    printf("[%20s/%3d]: \ttook: %.2fms \t avg: %.2fms, +-%fus\n", _name, _iters, duration/1000, average/1000, std::max(std::abs(average-min), std::abs(average-max)));
  }

 private:
  const char* _name;
  int32_t _iters;
  F _func{};
};

template <typename T>
class sum_pipeline : public pipeline_base<sum_pipeline<T>> {
   public:
    template<typename... V>
    void onReceive(V&&... value) {
        DoNotOptimize(_acc += (value + ...));
    }

    explicit sum_pipeline(T val) : _acc(val) {}

   private:
    T _acc;
};

/**
 * Pipe component for running a function for each element of a pipeline
 * @param function
 * @return itself
*/
template <typename T>
sum_pipeline<T> sum_r(T&& val) {
    return sum_pipeline<T>{val};
}

class bool_cycle : public gen::base_generator<bool_cycle, bool>{
    bool _b{true};
    public:
    IT(bool_cycle, bool);
    bool hasNext(){ return true; }
    bool next() { _b = !_b; return _b; }
    bool curr() { return _b; }
};

void range_s () {
  int64_t sum = 0;
  auto range_gen = gen::range(1000000LL);
  auto sum_range = sink::sum(sum);
  range_gen >>= sum_range;
  assert(sum == 499999500000LL);
}

extern "C" void app_main(void) {
  int32_t iters = 100000;
  benchmark range_many_ops("Range many ops", 10, [&]() {
    int64_t acc = 0, sum = 0;
    auto range_gen = gen::range(iters);
    
    auto transform_pipeline 
    =   pipe::transform([](auto i){ return i%13; })
    >>= pipe::set_state([](auto i, auto& _state){ _state = i + (_state%9); }, acc)
    >>= pipe::transform_s([](auto i, auto& _state){ return i + _state; }, acc);
    
    auto pipeline
    =   transform_pipeline
    >>= pipe::partition([](auto i){ return (i%13) == 0; },
        pipe::transform([](auto i){ return i + 13;})
        >>= sink::sum(sum),
        sink::hole()
    );
    range_gen >>= pipeline;
    assert(sum == 233337);
  });
  range_many_ops.run();

  benchmark loop_many_ops("Loop many ops", 10, [&]() {
    int64_t acc = 0, sum = 0;
    for (int64_t i = 0; i < iters; ++i){
        int64_t x = (i%13);
        acc = x + (acc % 9);
        x += acc;
        if ((x%13) == 0) {
            x += 13;
            sum += x;
        }
    }
    assert(sum == 233337);
  });
  loop_many_ops.run();

  benchmark range_every_snd("Range sum every 2nd", 10, []() {
    int64_t sum = 0;
    auto bool_gen = bool_cycle();
    auto range_gen = gen::range(100000LL);
    auto mux_gen = gen::mux(bool_gen, range_gen);
    auto sum_range = pipe::transform([](auto b, auto i){ return (i*b); }) >>= sink::sum(sum);
    mux_gen >>= sum_range;
    assert(sum == 2499950000LL);
  });
  range_every_snd.run();

  benchmark loop_every_snd("Loop sum every 2nd", 10, []() {
    int64_t sum = 0;
    bool keep = true;
    for (int64_t i = 0; i < 100000LL; ++i) {
      sum += (i * keep);
      keep = !keep;
    }
    assert(sum == 2499950000LL);
  });
  loop_every_snd.run();

  benchmark range_sum("Range sum", 10, []() {
    int64_t sum = 0;
    auto range_gen = gen::range(1000000LL);
    auto sum_range = sink::sum(sum);
    range_gen >>= sum_range;
    assert(sum == 499999500000LL);
  });
  range_sum.run();

  benchmark range_sum_s("Range sum_s", 10, range_s);
  range_sum_s.run();

  benchmark loop_sum("Loop sum", 10, []() {
    int64_t sum = 0;
    for (int64_t i = 0; i < 1000000LL; ++i) {
      sum += i;
    }
    assert(sum == 499999500000);
  });
  loop_sum.run();
}
