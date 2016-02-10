#include "hayai/hayai.hpp"

#include "OSPRayFixture.h"

using std::cout;
using std::endl;
using std::string;

BENCHMARK_F(OSPRayFixture, test1, 1, 100)
{
  renderer.renderFrame(fb, OSP_FB_COLOR | OSP_FB_ACCUM);
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
  cout << "                        default: ao1" << endl;

  cout << endl;
  cout << "    -s | --sampling-rate --> Specify the sampling rate for volumes."
       << endl;
  cout << "                             default: 0.125" << endl;

  cout << endl;
  cout << "    -tc | --tf-color --> Specify the next color to in the transfer"
       << " function for volumes. Each entry will add to the total list of"
       << " colors in the order they are specified." << endl;
  cout << "                              Format: R G B" << endl;
  cout << "                         Value Range: [0,1]" << endl;

  cout << endl;
  cout << "    -is | --surface --> Specify an isosurface at value: val " << endl;

  cout << endl;
  cout << "    -vp | --eye --> Specify the camera eye as: ex ey ez " << endl;

  cout << endl;
  cout << "    -vi | --gaze --> Specify the camera gaze point as: ix iy iz "
       << endl;

  cout << endl;
  cout << "    -vu | --up --> Specify the camera up as: ux uy uz " << endl;

  cout << endl;
  cout << "    -bg | --background --> Specify the background color: R G B "
       << endl;

  exit(0);
}

void parseCommandLine(int argc, const char *argv[])
{
  if (argc <= 1) {
    printUsageAndExit();
  }

  for (int i = 1; i < argc; ++i) {
    string arg = argv[i];
    if (arg == "-vp" || arg == "--eye") {
      auto &pos = OSPRayFixture::pos;
      pos.x = atof(argv[++i]);
      pos.y = atof(argv[++i]);
      pos.z = atof(argv[++i]);
      OSPRayFixture::customView = true;
    } else if (arg == "-vu" || arg == "--up") {
      auto &up = OSPRayFixture::up;
      up.x = atof(argv[++i]);
      up.y = atof(argv[++i]);
      up.z = atof(argv[++i]);
      OSPRayFixture::customView = true;
    } else if (arg == "-vi" || arg == "--gaze") {
      auto &at = OSPRayFixture::at;
      at.x = atof(argv[++i]);
      at.y = atof(argv[++i]);
      at.z = atof(argv[++i]);
      OSPRayFixture::customView = true;
    } else if (arg == "-r" || arg == "--renderer") {
      OSPRayFixture::renderer_type = argv[++i];
    } else if (arg == "-i" || arg == "--image") {
      OSPRayFixture::imageOutputFile = argv[++i];
    } else if (arg == "-w" || arg == "--width") {
      OSPRayFixture::width = atoi(argv[++i]);
    } else if (arg == "-h" || arg == "--height") {
      OSPRayFixture::height = atoi(argv[++i]);
    } else if (arg == "-s" || arg == "--sampling-rate") {
      OSPRayFixture::samplingRate = atof(argv[++i]);
    } else if (arg == "-tc" || arg == "--tf-color") {
      ospray::vec3f color;
      color.x = atof(argv[++i]);
      color.y = atof(argv[++i]);
      color.z = atof(argv[++i]);
      OSPRayFixture::tf_colors.push_back(color);
    } else if (arg == "-is" || arg == "--surface") {
      OSPRayFixture::isosurfaces.push_back(atof(argv[++i]));
    } else if (arg == "-bg" || arg == "--background") {
      ospray::vec3f &color = OSPRayFixture::bg_color;
      color.x = atof(argv[++i]);
      color.y = atof(argv[++i]);
      color.z = atof(argv[++i]);
    } else {
      OSPRayFixture::benchmarkModelFiles.push_back(arg);
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
