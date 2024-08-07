#pragma once

#include <string>
#include <chrono>
#include <iostream>

class Benchmark {
public:
    Benchmark(const std::string& name, bool print_on_destroy = true);
    ~Benchmark();

    double get_duration_ms() const;

    void print() const;
    void print(std::ostream& os) const;

private:
    bool print_on_destroy;
    std::string name;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
};
