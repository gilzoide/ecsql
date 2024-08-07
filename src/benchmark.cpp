#include "benchmark.hpp"

Benchmark::Benchmark(const std::string& name, bool print_on_destroy)
    : print_on_destroy(print_on_destroy)
    , name(name)
    , start(std::chrono::high_resolution_clock::now())
{
}

Benchmark::~Benchmark() {
    if (print_on_destroy) {
        print();
    }
}

double Benchmark::get_duration_ms() const {
    auto now = std::chrono::high_resolution_clock::now();
    return (now - start).count() / (std::chrono::high_resolution_clock::duration::period::den / 1000.0);
}

void Benchmark::print() const {
    print(std::cerr);
}

void Benchmark::print(std::ostream& os) const {
    os << "[Benchmark] "
        << name
        << ": "
        << get_duration_ms()
        << " ms"
        << std::endl;
}
