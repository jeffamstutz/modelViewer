
#include <hayai/hayai.hpp>

#include "ospray/ospray.h"
#include "common/miniSG/miniSG.h"

struct OSPRayFixture : public hayai::Fixture
{
  void SetUp() override;
  void TearDown() override;

  ospray::miniSG::Model sgModel;
  OSPRenderer    renderer;
  OSPCamera      camera;
  OSPModel       model;
  OSPFrameBuffer fb;

  OSPTransferFunction tf;

  static float samplingRate;

  static bool customView;
  static ospray::vec3f pos;
  static ospray::vec3f at;
  static ospray::vec3f up;

  static std::string renderer_type;
  static std::string benchmarkModelFile;
  static std::string imageOutputFile;

  static int width;
  static int height;
};
