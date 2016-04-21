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

#include <tuple>

#include <ospray_cpp/Camera.h>
#include <ospray_cpp/Model.h>
#include <ospray_cpp/Renderer.h>

using ParsedOSPObjects = std::tuple<ospcommon::box3f,
                                    ospray::cpp::Model,
                                    ospray::cpp::Renderer,
                                    ospray::cpp::Camera>;

template <typename RendererParser_T,
          typename CameraParser_T,
          typename SceneParser_T,
          typename LightsParser_T>
ParsedOSPObjects parseCommandLine(int ac, const char **&av)
{
  CameraParser_T cameraParser;
  cameraParser.parse(ac, av);
  auto camera = cameraParser.camera();

  RendererParser_T rendererParser;
  rendererParser.parse(ac, av);
  auto renderer = rendererParser.renderer();

  SceneParser_T sceneParser{rendererParser.renderer()};
  sceneParser.parse(ac, av);
  auto sgmodel = sceneParser.sgmodel();
  auto model   = sceneParser.model();

  LightsParser_T lightsParser(renderer);
  lightsParser.parse(ac, av);

  return std::make_tuple(sgmodel->getBBox(), model, renderer, camera);
}
