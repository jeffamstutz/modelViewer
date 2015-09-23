﻿#include "MSGViewer.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

#include <string>
using std::string;

// Static local helper functions //////////////////////////////////////////////

// helper function to write the rendered image as PPM file
static void writePPM(const char *fileName, const int sizeX, const int sizeY,
                     const uint32 *pixel)
{
  FILE *file = fopen(fileName, "wb");
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

// Scripting callback functions ///////////////////////////////////////////////

namespace chaiospray {

/*! add 1-float parameter to given object */
void ospSet1f(OSPObject _object, const string id, float x)
{
  ::ospSet1f(_object, id.c_str(), x);
}

/*! add 1-int parameter to given object */
void ospSet1i(OSPObject _object, const string &id, int32 x)
{
  ::ospSet1i(_object, id.c_str(), x);
}

/*! add a 2-float parameter to a given object */
void ospSet2f(OSPObject _object, const string &id, float x, float y)
{
  ::ospSet2f(_object, id.c_str(), x, y);
}

/*! add a 2-int parameter to a given object */
void ospSet2i(OSPObject _object, const string &id, int x, int y)
{
  ::ospSet2i(_object, id.c_str(), x, y);
}

/*! add 3-float parameter to given object */
void ospSet3f(OSPObject _object, const string &id, float x, float y, float z)
{
  ::ospSet3f(_object, id.c_str(), x, y, z);
}

/*! add 3-int parameter to given object */
void ospSet3i(OSPObject _object, const string &id, int x, int y, int z)
{
  ::ospSet3i(_object, id.c_str(), x, y, z);
}

/*! \brief commit changes to an object */
void ospCommit(OSPObject object)
{
  ::ospCommit(object);
}

}

// MSGViewer definitions //////////////////////////////////////////////////////

namespace ospray {

MSGViewer::MSGViewer(miniSG::Model *sgmodel, OSPModel model,
                     OSPRenderer renderer, OSPCamera camera,
                     ViewerConfig config)
  : Glut3DWidget(Glut3DWidget::FRAMEBUFFER_NONE),
    m_sgmodel(sgmodel),
    m_model(model),
    m_fb(NULL),
    m_renderer(renderer),
    m_camera(camera),
    m_config(config),
    m_accumID(-1),
    m_fullScreen(false),
    m_nearClip(1e-6f),
    m_maxDepth(2)
{
  const box3f worldBounds(m_sgmodel->getBBox());
  setWorldBounds(worldBounds);
  cout << "#ospModelViewer: set world bounds " << worldBounds
       << ", motion speed " << motionSpeed << endl;
  if (m_sgmodel->camera.size() > 0) {
    setViewPort(m_sgmodel->camera[0]->from,
        m_sgmodel->camera[0]->at,
        m_sgmodel->camera[0]->up);
  }

  registerScriptObjects();
  registerScriptFunctions();
}

void MSGViewer::reshape(const vec2i &newSize)
{
  Glut3DWidget::reshape(newSize);
  m_windowSize = newSize;
  if (m_fb) ospFreeFrameBuffer(m_fb);
  m_fb = ospNewFrameBuffer(newSize, OSP_RGBA_I8,
                           OSP_FB_COLOR|OSP_FB_DEPTH|OSP_FB_ACCUM);
  ospSet1f(m_fb, "gamma", 2.2f);
  ospCommit(m_fb);
  ospFrameBufferClear(m_fb,OSP_FB_ACCUM);
  ospSetf(m_camera,"aspect",viewPort.aspect);
  ospCommit(m_camera);
  viewPort.modified = true;
  forceRedraw();
}

void MSGViewer::keypress(char key, const vec2f where)
{
  switch (key) {
  case ':':
    cout << endl << "...pause rendering..." << endl << endl;
    cout << "**** MODEL VIEWER COMMAND MODE ****" << endl;
    getConsoleCommands();
    cout << endl << "...resume rendering..." << endl << endl;
    break;
  case 'R':
    m_config.alwaysRedraw = !m_config.alwaysRedraw;
    forceRedraw();
    break;
  case 'S':
    m_config.doShadows = !m_config.doShadows;
    cout << "Switching shadows " << (m_config.doShadows?"ON":"OFF") << endl;
    ospSet1i(m_renderer,"shadowsEnabled", m_config.doShadows);
    ospCommit(m_renderer);
    m_accumID=0;
    ospFrameBufferClear(m_fb,OSP_FB_ACCUM);
    forceRedraw();
    break;
  case 'D':
    m_config.showDepthBuffer = !m_config.showDepthBuffer;
    // switch(g_frameBufferMode) {
    //   case Glut3DWidget::FRAMEBUFFER_DEPTH:
    //     g_frameBufferMode = Glut3DWidget::FRAMEBUFFER_UCHAR;
    //     break;
    //   case Glut3DWidget::FRAMEBUFFER_UCHAR:
    //     g_frameBufferMode = Glut3DWidget::FRAMEBUFFER_DEPTH;
    //     break;
    // }
    ospFrameBufferClear(m_fb,OSP_FB_ACCUM);
    forceRedraw();
    break;
  case '!': {
    const uint32 * p = (uint32*)ospMapFrameBuffer(m_fb, OSP_FB_COLOR);
    writePPM("ospmodelviewer.ppm", m_windowSize.x, m_windowSize.y, p);
    // ospUnmapFrameBuffer(fb,p);
    cout << "#ospModelViewer: saved current frame to 'ospmodelviewer.ppm'"
         << endl;
  } break;
  case 'X':
    if (viewPort.up == vec3f(1,0,0) || viewPort.up == vec3f(-1.f,0,0))
      viewPort.up = - viewPort.up;
    else
      viewPort.up = vec3f(1,0,0);
    viewPort.modified = true;
    forceRedraw();
    break;
  case 'Y':
    if (viewPort.up == vec3f(0,1,0) || viewPort.up == vec3f(0,-1.f,0))
      viewPort.up = - viewPort.up;
    else
      viewPort.up = vec3f(0,1,0);
    viewPort.modified = true;
    forceRedraw();
    break;
  case 'Z':
    if (viewPort.up == vec3f(0,0,1) || viewPort.up == vec3f(0,0,-1.f))
      viewPort.up = - viewPort.up;
    else
      viewPort.up = vec3f(0,0,1);
    viewPort.modified = true;
    forceRedraw();
    break;
  case 'f':
    m_fullScreen = !m_fullScreen;
    if(m_fullScreen) glutFullScreen();
    else glutPositionWindow(0,10);
    break;
  case 'r':
    viewPort = m_viewPort;
    break;
  case 'p':
    printf("-vp %f %f %f -vu %f %f %f -vi %f %f %f\n",
           viewPort.from.x, viewPort.from.y, viewPort.from.z, viewPort.up.x,
           viewPort.up.y, viewPort.up.z, viewPort.at.x, viewPort.at.y,
           viewPort.at.z);
    fflush(stdout);
    break;
  default:
    Glut3DWidget::keypress(key,where);
  }
}

void ospray::MSGViewer::specialkey(int32 key, const vec2f where)
{
  switch(key) {
  case GLUT_KEY_PAGE_UP:
    m_nearClip += 20.f * motionSpeed;
  case GLUT_KEY_PAGE_DOWN:
    m_nearClip -= 10.f * motionSpeed;
    m_nearClip = std::max(m_nearClip, 1e-6f);
    ospSet1f(m_camera, "near_clip", m_nearClip);
    ospCommit(m_camera);
    m_accumID=0;
    ospFrameBufferClear(m_fb,OSP_FB_ACCUM);
    forceRedraw();
    break;
  case GLUT_KEY_HOME:
    m_maxDepth += 2;
  case GLUT_KEY_END:
    m_maxDepth--;
    ospSet1i(m_renderer, "maxDepth", m_maxDepth);
    PRINT(m_maxDepth);
    ospCommit(m_renderer);
    m_accumID=0;
    ospFrameBufferClear(m_fb,OSP_FB_ACCUM);
    forceRedraw();
    break;
  default:
    Glut3DWidget::specialkey(key,where);
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
    ospPick(&pick, m_renderer, normpos);
    if(pick.hit) {
      viewPort.at = pick.position;
      viewPort.modified = true;
      computeFrame();
      forceRedraw();
    }
  }
}

void MSGViewer::display()
{
  if (!m_fb || !m_renderer) return;

  static int frameID = 0;

  //{
  // note that the order of 'start' and 'end' here is
  // (intentionally) reversed: due to our asynchrounous rendering
  // you cannot place start() and end() _around_ the renderframe
  // call (which in itself will not do a lot other than triggering
  // work), but the average time between ttwo calls is roughly the
  // frame rate (including display overhead, of course)
  if (frameID > 0) m_fps.doneRender();
  m_fps.startRender();
  //}
  static double benchStart=0;
  static double fpsSum=0;
  if (m_config.benchFrames > 0 && frameID == m_config.benchWarmup)
    benchStart = ospray::getSysTime();
  if (m_config.benchFrames > 0 && frameID >= m_config.benchWarmup)
    fpsSum += m_fps.getFPS();
  if (m_config.benchFrames > 0 &&
      frameID== m_config.benchWarmup + m_config.benchFrames)
  {
    double time = ospray::getSysTime()-benchStart;
    double avgFps = fpsSum / double(frameID - m_config.benchWarmup);
    printf("Benchmark: time: %f avg fps: %f avg frame time: %f\n", time,
           avgFps, time / double(frameID - m_config.benchWarmup));

    const uint32 * p = (uint32*)ospMapFrameBuffer(m_fb, OSP_FB_COLOR);
    writePPM("benchmark.ppm", m_windowSize.x, m_windowSize.y, p);

    exit(0);
  }

  ++frameID;

  if (viewPort.modified) {
    static bool once = true;
    if(once) {
      m_viewPort = viewPort;
      once = false;
    }
    Assert2(m_camera,"ospray camera is null");
    ospSetVec3f(m_camera,"pos",viewPort.from);
    ospSetVec3f(m_camera,"dir",viewPort.at-viewPort.from);
    ospSetVec3f(m_camera,"up",viewPort.up);
    ospSetf(m_camera,"aspect",viewPort.aspect);
    ospCommit(m_camera);
    viewPort.modified = false;
    m_accumID=0;
    ospFrameBufferClear(m_fb,OSP_FB_ACCUM);
  }

  if (m_config.outFileName) {
    ospSet1i(m_renderer, "spp", m_config.numSPPinFileOutput);
    ospCommit(m_renderer);

    std::cout << "#ospModelViewer: Renderering offline image with "
              << m_config.numSPPinFileOutput
              << " samples per pixel per frame, and accumulation of "
              << m_config.numAccumsFrameInFileOutput << " such frames"
              << endl;

    for (int i = 0; i < m_config.numAccumsFrameInFileOutput; i++) {
      ospRenderFrame(m_fb,m_renderer,OSP_FB_COLOR|OSP_FB_ACCUM);
      ucharFB = (uint32 *) ospMapFrameBuffer(m_fb, OSP_FB_COLOR);
      std::cout << "#ospModelViewer: Saved rendered image (w/ " << i
                << " accums) in " << m_config.outFileName << std::endl;
      writePPM(m_config.outFileName, m_windowSize.x,
               m_windowSize.y, ucharFB);
      ospUnmapFrameBuffer(ucharFB, m_fb);
    }
    exit(0);
  }

  ospRenderFrame(m_fb, m_renderer, OSP_FB_COLOR |
                 (m_config.showDepthBuffer ? OSP_FB_DEPTH : 0) |
                 OSP_FB_ACCUM);
  ++m_accumID;

  // set the glut3d widget's frame buffer to the opsray frame buffer,
  // then display
  ucharFB = (uint32 *) ospMapFrameBuffer(m_fb, OSP_FB_COLOR);
  frameBufferMode = Glut3DWidget::FRAMEBUFFER_UCHAR;
  Glut3DWidget::display();

  // that pointer is no longer valid, so set it to null
  ucharFB = NULL;

  char title[1000];

  if (m_config.alwaysRedraw) {
    sprintf(title,"OSPRay Model Viewer (%f fps)",
            m_fps.getFPS());
    setTitle(title);
    forceRedraw();
  } else if (m_accumID < m_config.maxAccum) {
    forceRedraw();
  } else {
    // sprintf(title,"OSPRay Model Viewer");
    // setTitle(title);
  }
}

void MSGViewer::getConsoleCommands()
{
  string line;

  do {
    cout << "% ";

    getline(cin, line);

    while(line[0] == ' ') {
      line.erase(line.begin(), line.begin()++);
    }

    if (line == "done" || line == "exit") {
      break;
    }

    try {
      m_chai.eval(line);
    } catch (const chaiscript::exception::eval_error &e) {
      cerr << e.what() << endl;
    }
  } while (1);
}

void MSGViewer::registerScriptObjects()
{
  m_chai.add(chaiscript::var((osp::ManagedObject*)m_model),    "model"   );
  m_chai.add(chaiscript::var((osp::ManagedObject*)m_renderer), "renderer");
  m_chai.add(chaiscript::var((osp::ManagedObject*)m_camera),   "camera"  );
}

void MSGViewer::registerScriptFunctions()
{
  m_chai.add(chaiscript::fun(&MSGViewer::saySomething, this), "saySomething");

  // ospray functions //

  m_chai.add(chaiscript::fun(&chaiospray::ospSet1f),  "ospSet1f");
  m_chai.add(chaiscript::fun(&chaiospray::ospSet1i),  "ospSet1i");
  m_chai.add(chaiscript::fun(&chaiospray::ospSet2f),  "ospSet2f");
  m_chai.add(chaiscript::fun(&chaiospray::ospSet2f),  "ospSet2i");
  m_chai.add(chaiscript::fun(&chaiospray::ospSet3i),  "ospSet3f");
  m_chai.add(chaiscript::fun(&chaiospray::ospSet3i),  "ospSet3i");
  m_chai.add(chaiscript::fun(&chaiospray::ospCommit), "ospCommit");
}

void MSGViewer::saySomething()
{
  cout << "CALLBACK FUNCTION" << endl;
}

}// namepace ospray
