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

#include "QOSPRayConsole.h"

#include "common/script/OSPRayChaiUtil.h"
#include "common/script/chaiscript/chaiscript_stdlib.hpp"

QOSPRayConsole::QOSPRayConsole(QWidget *parent) :
  QConsole(parent),
  m_chai(chaiscript::Std_Lib::library())
{
  ospray::script::registerScriptTypes(m_chai);
  ospray::script::registerScriptFunctions(m_chai);

  this->setWindowTitle("Script Window");
}

void QOSPRayConsole::setOSPRayModel(ospray::cpp::Model model)
{
  m_model = model;
}

void QOSPRayConsole::setOSPRayRenderer(ospray::cpp::Renderer renderer)
{
  m_renderer = renderer;
}

void QOSPRayConsole::setOSPRayCamera(ospray::cpp::Camera camera)
{
  m_camera = camera;
}

QString QOSPRayConsole::interpretCommand(const QString &command, int &res)
{
  res = 0;

  QConsole::interpretCommand(command, res);

  ospray::script::registerScriptObjects(m_chai, m_model, m_renderer, m_camera);

  try {
    m_chai.eval(command.toStdString());
  } catch (const chaiscript::exception::eval_error &e) {
    res = 1;
    return e.what();
  }

  return "success";
}
