// ======================================================================== //
// Copyright 2009-2016 Intel Corporation                                    //
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

#include "QConsole.h"

#include <ospray_cpp/Camera.h>
#include <ospray_cpp/Model.h>
#include <ospray_cpp/Renderer.h>

// ChaiScript
#include "common/script/chaiscript/chaiscript.hpp"

class QOSPRayConsole : public QConsole
{
  Q_OBJECT

public:

  QOSPRayConsole(QWidget *parent = nullptr);

  void setOSPRayModel(ospray::cpp::Model model);
  void setOSPRayRenderer(ospray::cpp::Renderer renderer);
  void setOSPRayCamera(ospray::cpp::Camera camera);

private:

  //execute a validated command
  QString interpretCommand(const QString &command, int &res) override;

  ospray::cpp::Model    m_model;
  ospray::cpp::Renderer m_renderer;
  ospray::cpp::Camera   m_camera;

  chaiscript::ChaiScript m_chai;
};
