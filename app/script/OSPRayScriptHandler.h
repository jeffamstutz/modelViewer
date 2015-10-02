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

#include <thread>

#include "app/ospray_cpp.h"

// ChaiScript
#include "chaiscript/chaiscript.hpp"

namespace ospray {

class OSPRayScriptHandler
{
public:

  OSPRayScriptHandler(OSPModel model, OSPRenderer renderer, OSPCamera camera);
  ~OSPRayScriptHandler();

  void runScriptFromFile(const std::string &file);

  void start();
  void stop();

  bool running();

protected:

  //! \brief ChaiScript engine, only accessable if the interactive thread isn't
  //!        running.
  //!
  //! \note Allow anyone who extends (inherits from) OSPRayScriptHandler to
  //!       have access to the engine to let them add custom functions or types.
  chaiscript::ChaiScript &scriptEngine();

  //! \note Child classes should append this string with any additional help
  //!       text that is desired when 'help' is invoked in the script engine.
  std::string m_helpText;

private:

  void consoleLoop();

  void runChaiLine(const std::string &line);
  void runChaiFile(const std::string &file);

  void registerScriptObjects();
  void registerScriptTypes();
  void registerScriptFunctions();

  // Data //

  osp::cpp::Model    m_model;
  osp::cpp::Renderer m_renderer;
  osp::cpp::Camera   m_camera;

  chaiscript::ChaiScript m_chai;

  bool m_running;

  //! \brief background thread to handle the scripting commands from the console
  std::thread m_thread;
};

}// namespace ospray
