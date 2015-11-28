
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

  static std::string benchmarkModelFile;
};
