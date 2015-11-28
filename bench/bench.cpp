#include <hayai/hayai.hpp>

#include "OSPRayFixture.h"

BENCHMARK_F(OSPRayFixture, test1, 1, 100)
{
  ospRenderFrame(fb, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);
}

int main(int argc, char *argv[])
{
  if (argc > 1) {
    OSPRayFixture::benchmarkModelFile = argv[1];
  }

  hayai::ConsoleOutputter consoleOutputter;

  hayai::Benchmarker::AddOutputter(consoleOutputter);
  hayai::Benchmarker::RunAllTests();
  return 0;
}
