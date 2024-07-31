#include "benchmark.hpp"

#include <chrono>
#include <iostream>

Benchmark::Benchmark(const std::string& name)
    : name(name)
    , start(chrono::high_resolution_clock::now())
{
}

Benchmark::~Benchmark() {
    cerr << "[Benchmark] "
        << name
        << ": "
        << ((chrono::high_resolution_clock::now() - start).count() / (chrono::high_resolution_clock::duration::period::den / 1000.0))
        << " ms"
        << endl;
}
