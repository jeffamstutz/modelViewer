#include "DebugViewerScriptHandler.h"

#include "MSGViewer.h"

#include <iostream>
using std::endl;

namespace ospray {

DebugViewerScriptHandler::DebugViewerScriptHandler(OSPModel    model,
                                                   OSPRenderer renderer,
                                                   OSPCamera   camera,
                                                   MSGViewer  *viewer) :
  OSPRayScriptHandler(model, renderer, camera),
  m_viewer(viewer)
{
  registerScriptFunctions();

  std::stringstream ss;

  ss << "Viewer functions available" << endl << endl;
  ss << "setRenderer(renderer) --> set the renderer in the viewer" << endl;
  ss << "refresh()             --> reset the accumulation buffer" << endl;
  ss << "toggleFullScreen()    --> toggle fullscreen mode" << endl;
  ss << "resetView()           --> reset camera view" << endl;
  ss << "printViewport()       --> print view params in the console" << endl;

  m_helpText += ss.str();
}

void DebugViewerScriptHandler::registerScriptFunctions()
{
  auto &chai = this->scriptEngine();

  // setRenderer()
  auto setRenderer = [&](osp::cpp::Renderer &r) {
    m_viewer->setRenderer((OSPRenderer)r.handle());
  };

  // refresh()
  auto refresh = [&]() {
    m_viewer->resetAccumulation();
  };

  // toggleFullscreen()
  auto toggleFullscreen = [&]() {
    m_viewer->toggleFullscreen();
  };

  // resetView()
  auto resetView = [&]() {
    m_viewer->resetView();
  };

  // printViewport()
  auto printViewport = [&]() {
    m_viewer->printViewport();
  };

  chai.add(chaiscript::fun(setRenderer),      "setRenderer"     );
  chai.add(chaiscript::fun(refresh),          "refresh"         );
  chai.add(chaiscript::fun(toggleFullscreen), "toggleFullscreen");
  chai.add(chaiscript::fun(resetView),        "resetView"       );
  chai.add(chaiscript::fun(printViewport),    "printViewport"   );
}

}// namespace ospray
