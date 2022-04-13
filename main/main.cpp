#include <stdio.h>
#include <stdlib.h>
#include "driver/adc.h"
#include "dvfw/dvfw.hpp"

using namespace dvfw;

class sensor_gen : public gen::base_generator<int> {
    private:
     bool* _flag;
   public:
    bool hasNext() {
        return *_flag;
    }

    int next() {
        sensor_gen::wait(10);
        return hall_sensor_read();
    }

    sensor_gen(bool* flag): _flag{flag} {}
};

extern "C" void app_main(void) {
    bool flag = false;
    auto sensor_generator = sensor_gen(&flag);
    auto pipeline
    =   pipe::moving_avg<int, 10>()
    >>= pipe::transform([](float i){ return static_cast<int>(i); })
    >>= pipe::set_state([](bool& flag){ flag = false; }, flag)
    >>= sink::printf("%d\n"); 
    
    sensor_generator >>= pipeline;
}
