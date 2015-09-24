
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

  void start();
  void stop();

private:

  void consoleLoop();

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
