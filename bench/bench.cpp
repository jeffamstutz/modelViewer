#include <hayai/hayai.hpp>

#include "OSPRayFixture.h"

BENCHMARK_F(OSPRayFixture, test1, 1, 100)
{
  ospRenderFrame(fb, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);
}

void parseCommandLine(int argc, const char *argv[])
{
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-view" || arg == "--view") {
      assert(i+9 <= argc);

      auto &pos = OSPRayFixture::pos;
      pos.x = atof(argv[++i]);
      pos.y = atof(argv[++i]);
      pos.z = atof(argv[++i]);

      auto &up = OSPRayFixture::up;
      up.x = atof(argv[++i]);
      up.y = atof(argv[++i]);
      up.z = atof(argv[++i]);

      auto &at = OSPRayFixture::at;
      at.x = atof(argv[++i]);
      at.y = atof(argv[++i]);
      at.z = atof(argv[++i]);

      OSPRayFixture::customView = true;
    }
    else {
      OSPRayFixture::benchmarkModelFile = arg;
    }
  }
}

int main(int argc, const char *argv[])
{
  ospInit(&argc, argv);
  parseCommandLine(argc, argv);

  hayai::ConsoleOutputter consoleOutputter;

  hayai::Benchmarker::AddOutputter(consoleOutputter);
  hayai::Benchmarker::RunAllTests();
  return 0;
}
