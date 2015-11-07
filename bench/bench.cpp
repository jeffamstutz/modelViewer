
#include <random>
#include <vector>

#include <hayai/hayai.hpp>

#define NUM_INTS 10000

BENCHMARK(TestGroup, Test1, 10, 100)
{
  std::vector<int> numbers;

  numbers.reserve(NUM_INTS);

  std::default_random_engine rng;
  std::uniform_int_distribution<int> dist(0, 2*NUM_INTS);

  for (int i = 0; i < NUM_INTS; ++i) {
    numbers.push_back(dist(rng));
  }

  std::sort(numbers.begin(), numbers.end());
}

int main()
{
  hayai::ConsoleOutputter consoleOutputter;

  hayai::Benchmarker::AddOutputter(consoleOutputter);
  hayai::Benchmarker::RunAllTests();
  return 0;
}
