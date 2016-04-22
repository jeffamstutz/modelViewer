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

#include <common/commandline/CommandLineParser.h>
#include <ospray_cpp/Model.h>
#include <ospray_cpp/Renderer.h>
#include <common/miniSG/miniSG.h>

#include <string>

class SceneParser : public CommandLineParser
{
public:
  virtual ospray::cpp::Model     model()   = 0;
  virtual ospray::miniSG::Model* sgmodel() = 0;
};

class DefaultSceneParser : public SceneParser
{
public:
  DefaultSceneParser(ospray::cpp::Renderer);

  virtual void parse(int ac, const char **&av) override;

  ospray::cpp::Model     model();
  ospray::miniSG::Model* sgmodel();

protected:

  ospray::cpp::Material createDefaultMaterial(ospray::cpp::Renderer renderer);
  ospray::cpp::Material createMaterial(ospray::cpp::Renderer renderer,
                                       ospray::miniSG::Material *mat);

  ospray::cpp::Model    m_model;
  ospray::cpp::Renderer m_renderer;

  bool m_alpha;
  bool m_createDefaultMaterial;
  unsigned int m_maxObjectsToConsider;

  // if turned on, we'll put each triangle mesh into its own instance,
  // no matter what
  bool m_forceInstancing;

  ospcommon::Ref<ospray::miniSG::Model> m_msgModel;
  std::vector<ospray::miniSG::Model *> m_msgAnimation;

private:

  void createSpheres();
  void createCylinders();

  void finalize();
};
