#pragma once

#include <string>
#include <chrono>

using namespace std;

class Benchmark {
public:
    Benchmark(const std::string& name);
    ~Benchmark();

private:
    std::string name;
    chrono::time_point<chrono::high_resolution_clock> start;
};