// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

// viewer widget
#include "common/widgets/glut3D.h"
// mini scene graph for loading the model
#include "miniSG/miniSG.h"
// ospray, for rendering
#include "ospray/ospray.h"

#include "ViewerConfig.h"

#include "OSPRayScriptHandler.h"

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

  OSPRayScriptHandler m_scriptHandler;
};

}// namespace ospray
