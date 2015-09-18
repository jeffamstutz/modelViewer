
#pragma once

// viewer widget
#include "common/widgets/glut3D.h"

#include "miniSG/miniSG.h"

#include "ospray/ospray.h"

#include "ViewerConfig.h"

namespace ospray {

class CommandLineSceneBuilder
{
public:

  CommandLineSceneBuilder(int ac, const char **&av);

  miniSG::Model* sgmodel();
  OSPModel       model();
  OSPRenderer    renderer();
  OSPCamera      camera();
  ViewerConfig   viewerConfig();

private:

  // Helper functions //

  void parseCommandLine(int ac, const char **&av);
  void reportParsedData();
  void createScene();

  // Data //

  OSPModel    m_model;
  OSPRenderer m_renderer;
  OSPCamera   m_camera;

  ViewerConfig m_config;

  // Previously global data //

  Ref<miniSG::Model> msgModel;
  std::vector<miniSG::Model *> msgAnimation;

  /*! when using the OBJ renderer, we create a automatic dirlight with this
   * direction; use ''--sun-dir x y z' to change */
  vec3f defaultDirLight_direction;

  bool g_fullScreen;

  bool g_alpha;
  bool g_createDefaultMaterial;

  int spp; /*! number of samples per pixel */

  unsigned int maxObjectsToConsider;
  // if turned on, we'll put each triangle mesh into its own instance,
  // no matter what
  bool forceInstancing;
  /*! if turned on we're showing the depth buffer rather than the (accum'ed)
   *  color buffer */
  glut3D::Glut3DWidget::FrameBufferMode g_frameBufferMode;

  std::string rendererType;

  OSPMaterial createDefaultMaterial(OSPRenderer renderer);
  OSPMaterial createMaterial(OSPRenderer renderer, miniSG::Material *mat);
};

// Inlined function definitions ///////////////////////////////////////////////

inline miniSG::Model *CommandLineSceneBuilder::sgmodel()
{
  return msgModel.ptr;
}

inline OSPModel CommandLineSceneBuilder::model()
{
  return m_model;
}

inline OSPRenderer CommandLineSceneBuilder::renderer()
{
  return m_renderer;
}

inline OSPCamera CommandLineSceneBuilder::camera()
{
  return m_camera;
}

inline ViewerConfig CommandLineSceneBuilder::viewerConfig()
{
  return m_config;
}

}// namespace ospray
