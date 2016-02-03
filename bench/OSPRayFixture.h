
#include "hayai/hayai.hpp"

#include <ospray_cpp/Camera.h>
#include <ospray_cpp/Model.h>
#include <ospray_cpp/Renderer.h>
#include <ospray_cpp/TransferFunction.h>

#include "common/miniSG/miniSG.h"

struct OSPRayFixture : public hayai::Fixture
{
  OSPRayFixture();

  // Fixture hayai interface //

  void SetUp() override;
  void TearDown() override;

  // Fixture data //

  ospray::miniSG::Model sgModel;
  ospray::cpp::Renderer    renderer;
  ospray::cpp::Camera      camera;
  ospray::cpp::Model       model;
  ospray::cpp::FrameBuffer fb;
  ospray::cpp::TransferFunction tf;

  // Command-line configuration data //

  static float samplingRate;

  static bool customView;
  static ospray::vec3f pos;
  static ospray::vec3f at;
  static ospray::vec3f up;

  static std::string renderer_type;
  static std::string benchmarkModelFile;
  static std::string imageOutputFile;

  static std::vector<ospray::vec3f> tf_colors;

  static int width;
  static int height;

  static ospray::vec3f bg_color;
};
