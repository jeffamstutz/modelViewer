
#pragma once

// viewer widget
#include "common/widgets/glut3D.h"
// mini scene graph for loading the model
#include "miniSG/miniSG.h"
// ospray, for rendering
#include "ospray/ospray.h"

#include "ViewerConfig.h"

namespace ospray {

/*! mini scene graph viewer widget. \internal Note that all handling
  of camera is almost exactly similar to the code in volView;
  might make sense to move that into a common class! */
class MSGViewer : public ospray::glut3D::Glut3DWidget
{
public:

  MSGViewer(miniSG::Model *sgmodel, OSPModel model, OSPRenderer renderer,
            OSPCamera camera, ViewerConfig config = ViewerConfig());

private:

  void reshape(const ospray::vec2i &newSize);
  void keypress(char key, const vec2f where);
  void specialkey(int32 key, const vec2f where);
  void mouseButton(int32 whichButton, bool released, const vec2i &pos);

  void display();

  void getConsoleCommands();

  // Data //

  miniSG::Model *m_sgmodel;
  OSPModel       m_model;
  OSPFrameBuffer m_fb;
  OSPRenderer    m_renderer;
  OSPCamera      m_camera;
  ospray::glut3D::FPSCounter m_fps;

  ViewerConfig m_config;

  vec2i m_windowSize;
  int m_accumID;
  bool m_fullScreen;
  glut3D::Glut3DWidget::ViewPort m_viewPort;
  float m_nearClip;
  int m_maxDepth; // only set with home/end
};

}// namespace ospray
