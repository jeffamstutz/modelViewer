
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
  static ospcommon::vec3f pos;
  static ospcommon::vec3f at;
  static ospcommon::vec3f up;

  static std::string renderer_type;
  static std::string imageOutputFile;

  static std::vector<std::string> benchmarkModelFiles;

  static float tf_scale;
  static std::vector<ospcommon::vec3f> tf_colors;
  static std::vector<float> isosurfaces;

  static ospcommon::vec2f volume_data_range;

  static int width;
  static int height;

  static ospcommon::vec3f bg_color;
};
