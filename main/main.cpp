#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <iostream>
#include "dfl/dfl.hpp"
#include "InfInt.hpp"
#include <iterator>
#include <type_traits>

using namespace dfl;

class fibonacci : public gen::base_generator<InfInt> {
    InfInt _curr{0}, _next{1};
    public:
    fibonacci() {}
    bool hasNext() const { return true; }
    InfInt curr() const { return _curr; }
    void next() {
        InfInt temp = _next;
        _next += _curr;
        _curr = temp;
    }
    
    MAKE_ITER(fibonacci, InfInt);
};

extern "C" void app_main(void) {
    fibonacci() 
    >>= pipe::intersperse("\n")
    >>= sink::to_out_stream(std::cout);
}
