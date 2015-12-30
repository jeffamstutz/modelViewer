﻿#include "MSGViewer.h"

using std::cout;
using std::endl;

using std::string;

using std::lock_guard;
using std::mutex;

using ospray::uint32;

// Static local helper functions //////////////////////////////////////////////

// helper function to write the rendered image as PPM file
static void writePPM(const string &fileName, const int sizeX, const int sizeY,
                     const uint32 *pixel)
{
  FILE *file = fopen(fileName.c_str(), "wb");
  fprintf(file, "P6\n%i %i\n255\n", sizeX, sizeY);
  unsigned char *out = (unsigned char *)alloca(3*sizeX);
  for (int y = 0; y < sizeY; y++) {
    const unsigned char *in = (const unsigned char *)&pixel[(sizeY-1-y)*sizeX];
    for (int x = 0; x < sizeX; x++) {
      out[3*x + 0] = in[4*x + 0];
      out[3*x + 1] = in[4*x + 1];
      out[3*x + 2] = in[4*x +2 ];
    }
    fwrite(out, 3*sizeX, sizeof(char), file);
  }
  fprintf(file, "\n");
  fclose(file);
}

// MSGViewer definitions //////////////////////////////////////////////////////

namespace ospray {

MSGViewer::MSGViewer(miniSG::Model *sgmodel, cpp::Model model,
                     cpp::Renderer renderer, cpp::Camera camera,
                     ViewerConfig config)
  : Glut3DWidget(Glut3DWidget::FRAMEBUFFER_NONE),
    m_sgmodel(sgmodel),
    m_model(model),
    m_fb(nullptr),
    m_renderer(renderer),
    m_camera(camera),
    m_queuedRenderer(nullptr),
    m_config(config),
    m_accumID(-1),
    m_fullScreen(false),
    m_scriptHandler((OSPModel)model.handle(),
                    (OSPRenderer)renderer.handle(),
                    (OSPCamera)camera.handle(),
                    this)
{
  const box3f worldBounds(m_sgmodel->getBBox());
  setWorldBounds(worldBounds);

  if (m_config.verboseOutput) {
    cout << "#ospDebugViewer: set world bounds " << worldBounds
         << ", motion speed " << motionSpeed << endl;
  }

  if (m_sgmodel->camera.size() > 0) {
    setViewPort(m_sgmodel->camera[0]->from,
                m_sgmodel->camera[0]->at,
                m_sgmodel->camera[0]->up);
  }

  if (!m_config.scriptFileName.empty()) {
    m_scriptHandler.runScriptFromFile(m_config.scriptFileName);
  }

  m_resetAccum = false;
}

void MSGViewer::setRenderer(OSPRenderer renderer)
{
  lock_guard<mutex> lock{m_rendererMutex};
  (void)lock;// NOTE(jda) - squash "unused variable" warning...

  m_queuedRenderer = renderer;
}

void MSGViewer::resetAccumulation()
{
  m_resetAccum = true;
}

void MSGViewer::toggleFullscreen()
{
  m_fullScreen = !m_fullScreen;
  if(m_fullScreen) {
    glutFullScreen();
  } else {
    glutPositionWindow(0,10);
  }
}

void MSGViewer::resetView()
{
  viewPort = m_viewPort;
}

void MSGViewer::printViewport()
{
  printf("-vp %f %f %f -vu %f %f %f -vi %f %f %f\n",
         viewPort.from.x, viewPort.from.y, viewPort.from.z,
         viewPort.up.x,   viewPort.up.y,   viewPort.up.z,
         viewPort.at.x,   viewPort.at.y,   viewPort.at.z);
  fflush(stdout);
}

void MSGViewer::saveScreenshot(const std::string &basename)
{
  const uint32 *p = (uint32*)m_fb.map(OSP_FB_COLOR);
  writePPM(basename + ".ppm", m_windowSize.x, m_windowSize.y, p);
  cout << "#ospDebugViewer: saved current frame to '" << basename << ".ppm'"
       << endl;
}

void MSGViewer::reshape(const vec2i &newSize)
{
  Glut3DWidget::reshape(newSize);
  m_windowSize = newSize;
  m_fb = cpp::FrameBuffer(osp::vec2i{newSize.x, newSize.y}, OSP_RGBA_I8,
                          OSP_FB_COLOR | OSP_FB_DEPTH | OSP_FB_ACCUM);

  m_fb.set("gamma", 2.2f);
  m_fb.commit();
  m_fb.clear(OSP_FB_ACCUM);
  m_camera.set("aspect", viewPort.aspect);
  m_camera.commit();
  viewPort.modified = true;
  forceRedraw();
}

void MSGViewer::keypress(char key, const vec2f where)
{
  switch (key) {
  case ':':
    if (!m_scriptHandler.running()) {
      m_scriptHandler.start();
    }
    break;
  case 'R':
    m_config.alwaysRedraw = !m_config.alwaysRedraw;
    forceRedraw();
    break;
  case 'S':
    m_config.doShadows = !m_config.doShadows;
    cout << "Switching shadows " << (m_config.doShadows?"ON":"OFF") << endl;
    m_renderer.set("shadowsEnabled", m_config.doShadows);
    m_renderer.commit();
    m_accumID = 0;
    m_fb.clear(OSP_FB_ACCUM);
    forceRedraw();
    break;
  case '!':
    saveScreenshot("ospdebugviewer");
    break;
  case 'X':
    if (viewPort.up == vec3f(1,0,0) || viewPort.up == vec3f(-1.f,0,0)) {
      viewPort.up = - viewPort.up;
    } else {
      viewPort.up = vec3f(1,0,0);
    }
    viewPort.modified = true;
    forceRedraw();
    break;
  case 'Y':
    if (viewPort.up == vec3f(0,1,0) || viewPort.up == vec3f(0,-1.f,0)) {
      viewPort.up = - viewPort.up;
    } else {
      viewPort.up = vec3f(0,1,0);
    }
    viewPort.modified = true;
    forceRedraw();
    break;
  case 'Z':
    if (viewPort.up == vec3f(0,0,1) || viewPort.up == vec3f(0,0,-1.f)) {
      viewPort.up = - viewPort.up;
    } else {
      viewPort.up = vec3f(0,0,1);
    }
    viewPort.modified = true;
    forceRedraw();
    break;
  case 'f':
    toggleFullscreen();
    break;
  case 'r':
    resetView();
    break;
  case 'p':
    printViewport();
    break;
  default:
    Glut3DWidget::keypress(key,where);
  }
}

void MSGViewer::mouseButton(int32 whichButton, bool released, const vec2i &pos)
{
  Glut3DWidget::mouseButton(whichButton, released, pos);
  if((currButtonState ==  (1<<GLUT_LEFT_BUTTON)) &&
     (glutGetModifiers() & GLUT_ACTIVE_SHIFT)    &&
     (manipulator == inspectCenterManipulator)) {
    vec2f normpos = vec2f(pos.x / (float)windowSize.x,
                          1.0f - pos.y / (float)windowSize.y);
    OSPPickResult pick;
    ospPick(&pick, (OSPRenderer)m_renderer.handle(),
            reinterpret_cast<osp::vec2f&>(normpos));
    if(pick.hit) {
      viewPort.at = reinterpret_cast<ospray::vec3f&>(pick.position);
      viewPort.modified = true;
      computeFrame();
      forceRedraw();
    }
  }
}

void MSGViewer::display()
{
  if (!m_fb.handle() || !m_renderer.handle()) return;

  static int frameID = 0;

  //{
  // note that the order of 'start' and 'end' here is
  // (intentionally) reversed: due to our asynchrounous rendering
  // you cannot place start() and end() _around_ the renderframe
  // call (which in itself will not do a lot other than triggering
  // work), but the average time between the two calls is roughly the
  // frame rate (including display overhead, of course)
  if (frameID > 0) m_fps.doneRender();

  // NOTE: consume a new renderer if one has been queued by another thread
  switchRenderers();

  if (m_resetAccum) {
    m_fb.clear(OSP_FB_ACCUM);
    m_resetAccum = false;
  }

  m_fps.startRender();
  //}

  ++frameID;

  if (viewPort.modified) {
    static bool once = true;
    if(once) {
      m_viewPort = viewPort;
      once = false;
    }
    Assert2(m_camera,"ospray camera is null");
    m_camera.set("pos", viewPort.from);
    auto dir = viewPort.at - viewPort.from;
    m_camera.set("dir", dir);
    m_camera.set("up", viewPort.up);
    m_camera.set("aspect", viewPort.aspect);
    m_camera.commit();
    viewPort.modified = false;
    m_accumID=0;
    m_fb.clear(OSP_FB_ACCUM);
  }

  m_renderer.renderFrame(m_fb, OSP_FB_COLOR | OSP_FB_ACCUM);
  ++m_accumID;

  // set the glut3d widget's frame buffer to the opsray frame buffer,
  // then display
  ucharFB = (uint32 *)m_fb.map(OSP_FB_COLOR);
  frameBufferMode = Glut3DWidget::FRAMEBUFFER_UCHAR;
  Glut3DWidget::display();

  m_fb.unmap(ucharFB);

  // that pointer is no longer valid, so set it to null
  ucharFB = nullptr;

  std::string title("OSPRay Debug Viewer");

  if (m_config.alwaysRedraw) {
    title += " (" + std::to_string(m_fps.getFPS()) + " fps)";
    setTitle(title);
    forceRedraw();
  } else {
    setTitle(title);
  }
}

void MSGViewer::switchRenderers()
{
  lock_guard<mutex> lock{m_rendererMutex};
  (void)lock;// NOTE(jda) - squash "unused variable" warning...

  if (m_queuedRenderer.handle()) {
    m_renderer = m_queuedRenderer;
    m_queuedRenderer = nullptr;
    m_fb.clear(OSP_FB_ACCUM);
  }
}

}// namepace ospray
