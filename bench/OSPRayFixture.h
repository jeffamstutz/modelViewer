
#include "hayai/hayai.hpp"

#include "ospray/ospray.h"
#include "common/miniSG/miniSG.h"

struct OSPRayFixture : public hayai::Fixture
{
  // Fixture hayai interface //

  void SetUp() override;
  void TearDown() override;

  // Fixture data //

  ospray::miniSG::Model sgModel;
  OSPRenderer    renderer;
  OSPCamera      camera;
  OSPModel       model;
  OSPFrameBuffer fb;
  OSPTransferFunction tf;

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
