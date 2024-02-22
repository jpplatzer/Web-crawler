#include <gtest/gtest.h>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <limits>

void seed_random() {
    using namespace std::chrono;
    using ui = unsigned int;
    high_resolution_clock::time_point t = high_resolution_clock::now();
    ui lst = static_cast<ui>(t.time_since_epoch().count() % 
        std::numeric_limits<ui>::max());
    std::srand(lst);
}

GTEST_API_ int main(int argc, char **argv) {
    seed_random();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
