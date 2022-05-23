#include <stdio.h>
#include <stdlib.h>
#include "driver/adc.h"
#include "dfl/dfl.hpp"

using namespace dvfw;

template <typename Gen, typename T>
class GenIt : public std::iterator<std::input_iterator_tag, T, bool, T*, T&> {
 public:
  Gen* _gen;

  GenIt(Gen* gen) : _gen{gen} {};
  GenIt& operator++() {
    _gen->next();
    return *this;
  }

  bool operator!=(GenIt& other) { return _gen->hasNext(); }

  bool operator==(GenIt& other) { return !(_gen->hasNext()); }

  T operator*() { return _gen->curr(); }
};

template <typename T>
class bg {
  using Iterator = GenIt<bg<T>, T>;

 public:
 using value_type = T;
  virtual bool hasNext() = 0;
  virtual T next() = 0;
  virtual T curr() = 0;
  virtual Iterator begin() { return Iterator(this); }
  virtual Iterator end() { return Iterator(nullptr); }
};

template <typename T>
class range : public bg<T> {
 private:
  T _min, _max, _step;

 public:
  T _itVal;
  range() = default;
  range(T max) : _min{0}, _max{max}, _step{1}, _itVal{0} {};
  range(T min, T max) : _min{min}, _max{max}, _step{1}, _itVal{min} {};
  range(T min, T max, T step)
      : _min{min}, _max{max}, _step{step}, _itVal{min} {};

  bool hasNext() { return _max > _itVal; }

  T next() {
    _itVal += _step;
    return _itVal;
  }

  T curr() { return _itVal; }
};

template <class Gen>
class take : public bg<typename Gen::value_type> {
 private:
  Gen _gen;
  size_t _num;
  size_t _taken{0};

 public:
  using value_type = typename Gen::value_type;
  using T = value_type;

  value_type _itVal = _gen._itVal;

  take(size_t num, Gen generator) : _gen{generator}, _num{num} {}

  bool hasNext() { return _gen.hasNext() && (_taken < _num); }

  T next() {
    _taken++;
    _gen.next();
    _itVal = _gen._itVal;
    return _gen._itVal;
  }

  T curr(){
    return _itVal;
  }


};

extern "C" void app_main(void) {
    auto range_gen = take(10, range(100));
    auto pipeline
    =   pipe::moving_avg<int, 10>()
    >>= pipe::transform([](float i){ return static_cast<int>(i); })
    >>= sink::printf("%d\n"); 
    
    range_gen >>= pipeline;
}
