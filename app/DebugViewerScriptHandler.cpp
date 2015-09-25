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

#if 0
  auto setRenderer = [this](osp::cpp::Renderer &r) {
    m_viewer->setRenderer((OSPRenderer)r.handle());
  };

  chai.add(chaiscript::fun(setRenderer), "setRenderer");
#else
  chai.add(chaiscript::fun(&DebugViewerScriptHandler::setRenderer, this),
           "setRenderer");
#endif
}

void DebugViewerScriptHandler::setRenderer(osp::cpp::Renderer &renderer)
{
  m_viewer->setRenderer((OSPRenderer)renderer.handle());
}

}// namespace ospray
