#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <iostream>
#include "dfl/dfl.hpp"
#include "InfInt.hpp"

using namespace dfl;

class fibonacci : public gen::base_generator<fibonacci, InfInt> {
    InfInt _curr{0}, _next{1};
    public:
    fibonacci() {}
    IT(fibonacci, InfInt);
    InfInt curr() { return _curr; }
    InfInt next() {
        InfInt temp = _next;
        _next += _curr;
        _curr = temp;
        return _curr;
    }
    bool hasNext() {
        return true;
    }
};

extern "C" void app_main(void) {
    std::string nums[] = {"█","▓","▒","░"," "};
    auto fib_gen = fibonacci();
    fib_gen >>= sink::for_each([&nums](auto num){
        std::string curr;
        for(char c : num.toString()){
            curr.append(nums[((c-'0')/2)]);
        }
        std::cout << curr << '\n';
    });
}
