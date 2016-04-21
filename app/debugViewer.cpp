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

#include "common/commandline/CameraParser.h"
#include "common/commandline/LightsParser.h"
#include "common/commandline/RendererParser.h"
#include "common/commandline/SceneParser.h"
#include "MSGViewer.h"

std::string parseForScriptFile(int ac, const char **&av)
{
  std::string scriptFileName;

  for (int i = 1; i < ac; i++) {
    const std::string arg = av[i];
    if (arg == "--script" || arg == "-s") {
      scriptFileName = av[++i];
    }
  }

  return scriptFileName;
}

int main(int ac, const char **av)
{
  ospInit(&ac,av);
  ospray::glut3D::initGLUT(&ac,av);

  CameraParser cameraParser;
  cameraParser.parse(ac, av);
  auto camera = cameraParser.camera();

  RendererParser rendererParser;
  rendererParser.parse(ac, av);
  auto renderer = rendererParser.renderer();

  SceneParser sceneParser{rendererParser.renderer()};
  sceneParser.parse(ac, av);
  auto sgmodel = sceneParser.sgmodel();
  auto model   = sceneParser.model();

  LightsParser lightsParser(renderer);
  lightsParser.parse(ac, av);

  auto scriptFileName = parseForScriptFile(ac, av);

  ospray::MSGViewer window(sgmodel->getBBox(), model, renderer,
                           camera, scriptFileName);
  window.create("ospDebugViewer: OSPRay Mini-Scene Graph test viewer");

  ospray::glut3D::runGLUT();
}
