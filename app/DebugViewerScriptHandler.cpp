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
  chaiscript::ChaiScript &chai = this->scriptEngine();

  // setRenderer()
  auto setRenderer = [this](osp::cpp::Renderer &r) {
    m_viewer->setRenderer((OSPRenderer)r.handle());
  };

  chai.add(chaiscript::fun(setRenderer), "setRenderer");
}

}// namespace ospray
