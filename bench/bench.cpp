#include <hayai/hayai.hpp>

#include "OSPRayFixture.h"

BENCHMARK_F(OSPRayFixture, test1, 1, 100)
{
  ospRenderFrame(fb, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);
}

int main()
{
  hayai::ConsoleOutputter consoleOutputter;

  hayai::Benchmarker::AddOutputter(consoleOutputter);
  hayai::Benchmarker::RunAllTests();
  return 0;
}
