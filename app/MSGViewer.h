
#pragma once

// viewer widget
#include "common/widgets/glut3D.h"
// mini scene graph for loading the model
#include "miniSG/miniSG.h"
// ospray, for rendering
#include "ospray/ospray.h"

namespace ospray {

/*! mini scene graph viewer widget. \internal Note that all handling
  of camera is almost exactly similar to the code in volView;
  might make sense to move that into a common class! */
class MSGViewer : public ospray::glut3D::Glut3DWidget
{
public:

  MSGViewer(int ac, const char **&av);

private:

  void parseCommandLine(int ac, const char **&av);
  void reportParsedData();
  void createScene();

  void reshape(const ospray::vec2i &newSize);
  void keypress(char key, const vec2f where);
  void specialkey(int32 key, const vec2f where);
  void mouseButton(int32 whichButton, bool released, const vec2i &pos);

  void display();

  // Data //

  OSPModel       m_model;
  OSPFrameBuffer m_fb;
  OSPRenderer    m_renderer;
  OSPCamera      m_camera;
  ospray::glut3D::FPSCounter m_fps;

  // Previously global data //

  Ref<miniSG::Model> msgModel;
  std::vector<miniSG::Model *> msgAnimation;

  /*! when using the OBJ renderer, we create a automatic dirlight with this
   * direction; use ''--sun-dir x y z' to change */
  vec3f defaultDirLight_direction;

  bool doShadows;

  float g_near_clip;
  bool  g_fullScreen;
  glut3D::Glut3DWidget::ViewPort g_viewPort;

  vec2i g_windowSize;

  int g_benchWarmup;
  int g_benchFrames;
  bool g_alpha;
  bool g_createDefaultMaterial;

  int spp; /*! number of samples per pixel */
  bool alwaysRedraw;

  int accumID;
  int maxAccum;
  int maxDepth; // only set with home/end
  unsigned int maxObjectsToConsider;
  // if turned on, we'll put each triangle mesh into its own instance,
  // no matter what
  bool forceInstancing;
  /*! if turned on we're showing the depth buffer rather than the (accum'ed)
   *  color buffer */
  bool showDepthBuffer;
  glut3D::Glut3DWidget::FrameBufferMode g_frameBufferMode;

  const char *outFileName;

  size_t numAccumsFrameInFileOutput;
  size_t numSPPinFileOutput;

  std::string rendererType;

  OSPMaterial createDefaultMaterial(OSPRenderer renderer);
  OSPMaterial createMaterial(OSPRenderer renderer, miniSG::Material *mat);
};

}// namespace ospray
