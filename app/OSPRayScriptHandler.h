
#pragma once

#include <thread>

#include "ospray/ospray.h"

// ChaiScript
#include <chaiscript/chaiscript.hpp>

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
  void registerScriptFunctions();

  // Script callback functions //

  void saySomething();

  // Data //

  OSPModel       m_model;
  OSPRenderer    m_renderer;
  OSPCamera      m_camera;

  chaiscript::ChaiScript m_chai;

  bool m_running;

  //! \brief background thread to handle the scripting commands from the console
  std::thread m_thread;
};

}// namespace ospray
