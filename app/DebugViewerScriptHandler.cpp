#include "DebugViewerScriptHandler.h"

#include "MSGViewer.h"

namespace ospray {

DebugViewerScriptHandler::DebugViewerScriptHandler(OSPModel    model,
                                                   OSPRenderer renderer,
                                                   OSPCamera   camera,
                                                   MSGViewer  *viewer) :
  OSPRayScriptHandler(model, renderer, camera),
  m_viewer(viewer)
{
  registerScriptFunctions();
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
