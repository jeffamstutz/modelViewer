#include <hayai/hayai.hpp>

#include "OSPRayFixture.h"

using std::cout;
using std::endl;
using std::string;

BENCHMARK_F(OSPRayFixture, test1, 1, 100)
{
  ospRenderFrame(fb, renderer, OSP_FB_COLOR | OSP_FB_ACCUM);
}

void printUsageAndExit()
{
  cout << "Usage: ospBenchmark [options] model_file" << endl;

  cout << endl << "Args:" << endl;

  cout << endl;
  cout << "    model_file --> Scene used for benchmarking, supported types"
            << " are:" << endl;
  cout << "                   stl, msg, tri, xml, obj, hbp, x3d" << endl;

  cout << endl;
  cout << "Options:" << endl;

  cout << endl;
  cout << "    -i | --image --> Specify the base filename to write the"
       << " framebuffer to a file." << endl;
  cout << "                     If ommitted, no file will be written." << endl;
  cout << "                     NOTE: this option adds '.ppm' to the end of the"
       << " filename" << endl;

  cout << endl;
  cout << "    -w | --width --> Specify the width of the benchmark frame"
       << endl;
  cout << "                     default: 1024" << endl;

  cout << endl;
  cout << "    -h | --height --> Specify the height of the benchmark frame"
       << endl;
  cout << "                      default: 1024" << endl;

  cout << endl;
  cout << "    -r | --renderer --> Specify the renderer to be benchmarked."
       << endl;
  cout << "                        Ex: -r pathtracer" << endl;

  cout << endl;
  cout << "    -v | --view --> Specify the camera view as: ex ey ez ux uy uz"
       << " dx dy dz." << endl;
  cout << "                    Ex: -v 1.0 0 0 0 1.0 0 -1.0 0 0" << endl;

  exit(0);
}

void parseCommandLine(int argc, const char *argv[])
{
  if (argc <= 1) {
    printUsageAndExit();
  }

  for (int i = 1; i < argc; ++i) {
    string arg = argv[i];
    if (arg == "-v" || arg == "--view") {
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
    else if (arg == "-r" || arg == "--renderer") {
      OSPRayFixture::renderer_type = argv[++i];
    }
    else if (arg == "-i" || arg == "--image") {
      OSPRayFixture::imageOutputFile = argv[++i];
    } else if (arg == "-w" || arg == "--width") {
      OSPRayFixture::width = atoi(argv[++i]);
    } else if (arg == "-h" || arg == "--height") {
      OSPRayFixture::height = atoi(argv[++i]);
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
