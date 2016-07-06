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

#include <string>

#include "QOSPRayWindow.h"
#include "QChaiConsole.h"

QOSPRayWindow::QOSPRayWindow(QMainWindow *parent,
                             ospray::cpp::Renderer _renderer,
                             bool showFrameRate) :
  parent(parent),
  showFrameRate(showFrameRate),
  renderingEnabled(false),
  rotationRate(0.f),
  renderer(_renderer)
{
  // setup camera
  camera = ospray::cpp::Camera("perspective");

  camera.commit();

  renderer.set("camera", camera);
  renderer.set("backgroundEnabled", 1);
  renderer.commit();

  // connect signals and slots
  connect(&renderTimer, SIGNAL(timeout()), this, SLOT(updateGL()));
  connect(&renderRestartTimer, SIGNAL(timeout()), &renderTimer, SLOT(start()));

  this->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
          this, SLOT(showContextMenu(const QPoint &)));
}

QOSPRayWindow::~QOSPRayWindow()
{
  if(camera.handle())
    ospRelease(camera.handle());
}

void QOSPRayWindow::setRenderingEnabled(bool renderingEnabled)
{
  this->renderingEnabled = renderingEnabled;

  // trigger render if true
  if(renderingEnabled == true)
    renderTimer.start();
  else
    renderTimer.stop();
}

void QOSPRayWindow::setRotationRate(float rotationRate)
{
  this->rotationRate = rotationRate;
}

void QOSPRayWindow::setWorldBounds(const ospcommon::box3f &worldBounds)
{
  this->worldBounds = worldBounds;

  // set viewport look at point to center of world bounds
  viewport.at = ospcommon::center(worldBounds);

  auto diag = worldBounds.size();
  viewport.from = viewport.at - .75f * ospcommon::vec3f(-.6   * diag.x,
                                                        -1.2f * diag.y,
                                                        .8f   * diag.z);

  viewport.modified = true;
  viewport.snapUp();

  // NOTE(jda) - this is a horrible hack to deal with view initialization issues

  dolly(1.f);
  rotateCenter(1.f, 1.f);

  updateGL();
}

void QOSPRayWindow::showContextMenu(const QPoint &pos)
{
  QMenu contextMenu(tr("Context menu"), this);

  QAction closeAction("Close Window", this);
  connect(&closeAction, SIGNAL(triggered()), this, SLOT(close()));
  contextMenu.addAction(&closeAction);

  contextMenu.addSeparator();

  QAction consoleAction("Open Console", this);
  connect(&consoleAction, SIGNAL(triggered()), this, SLOT(showConsole()));
  contextMenu.addAction(&consoleAction);

  contextMenu.exec(mapToGlobal(pos));
}

void QOSPRayWindow::showConsole()
{
  static QChaiConsole console;
  console.show();
}

void QOSPRayWindow::resetAccumulationBuffer()
{
  frameBuffer.clear(OSP_FB_ACCUM);
}

void QOSPRayWindow::paintGL()
{
  if(!renderingEnabled || !frameBuffer.handle())
    return;

  // update OSPRay camera if viewport has been modified
  if(viewport.modified) {
    const ospcommon::vec3f dir =  viewport.at - viewport.from;
    camera.set("pos", viewport.from);
    camera.set("dir", dir);
    camera.set("up", viewport.up);
    camera.set("aspect", viewport.aspect);
    camera.set("fovy", viewport.fovY);

    camera.commit();

    viewport.modified = false;
  }

  renderFrameTimer.start();

  renderer.commit();

  renderer.renderFrame(frameBuffer, OSP_FB_COLOR | OSP_FB_ACCUM);
  double framesPerSecond = 1000.0 / renderFrameTimer.elapsed();
  char title[1024];
  sprintf(title, "OSPRay Debug Viewer (%.4f fps)", framesPerSecond);

  if (showFrameRate == true)
    this->setWindowTitle(title);

  uint32_t *mappedFrameBuffer = (unsigned int *)frameBuffer.map(OSP_FB_COLOR);

  glDrawPixels(windowSize.x, windowSize.y,
               GL_RGBA, GL_UNSIGNED_BYTE, mappedFrameBuffer);

  frameBuffer.unmap(mappedFrameBuffer);

  // automatic rotation
  if(rotationRate != 0.f) {
    resetAccumulationBuffer();
    rotateCenter(rotationRate, 0.f);
  }
}

void QOSPRayWindow::resizeGL(int width, int height)
{
  windowSize = ospcommon::vec2i(width, height);

  frameBuffer = ospray::cpp::FrameBuffer((const osp::vec2i&)windowSize,
                                         OSP_FB_SRGBA,
                                         OSP_FB_COLOR | OSP_FB_ACCUM);

  resetAccumulationBuffer();

  // update viewport aspect ratio
  viewport.aspect = float(width) / float(height);
  viewport.modified = true;

  updateGL();
}

void QOSPRayWindow::mousePressEvent(QMouseEvent *event)
{
  lastMousePosition = event->pos();
}

void QOSPRayWindow::mouseReleaseEvent(QMouseEvent *event)
{
  lastMousePosition = event->pos();

  // restart continuous rendering immediately
  renderTimer.start();
}

void QOSPRayWindow::mouseMoveEvent(QMouseEvent *event)
{
  stopRenderTimers();

  resetAccumulationBuffer();

  int dx = event->x() - lastMousePosition.x();
  int dy = event->y() - lastMousePosition.y();

  bool leftButton = event->buttons() & Qt::LeftButton;
  bool midButton  = event->buttons() & Qt::MidButton;
  bool controlKey = event->modifiers() == Qt::ControlModifier;

  if(leftButton && !controlKey) {
    // camera rotation about center point
    const float rotationSpeed = 0.003f;

    float du = dx * rotationSpeed;
    float dv = dy * rotationSpeed;

    rotateCenter(du, dv);
  } else if(leftButton && controlKey) {
    // camera strafe of from / at point
    const float strafeSpeed = 0.001f * length(worldBounds.size());

    float du = dx * strafeSpeed;
    float dv = dy * strafeSpeed;

    strafe(du, dv);
  } else if(midButton) {
    dolly(dy);
  }

  lastMousePosition = event->pos();

  updateGL();

  startRenderTimers();
}

void QOSPRayWindow::wheelEvent(QWheelEvent *event)
{
  stopRenderTimers();

  resetAccumulationBuffer();

  dolly(event->delta() / 10.f);

  updateGL();

  startRenderTimers();
}

void QOSPRayWindow::closeEvent(QCloseEvent *e)
{
  QGLWidget::closeEvent(e);
  emit closing();
}

void QOSPRayWindow::rotateCenter(float du, float dv)
{
  const ospcommon::vec3f pivot = viewport.at;

  ospcommon::affine3f xfm = ospcommon::affine3f::translate(pivot)
    * ospcommon::affine3f::rotate(viewport.frame.l.vx, -dv)
    * ospcommon::affine3f::rotate(viewport.frame.l.vz, -du)
    * ospcommon::affine3f::translate(-pivot);

  viewport.frame = xfm * viewport.frame;
  viewport.from  = xfmPoint(xfm, viewport.from);
  viewport.at    = xfmPoint(xfm, viewport.at);
  viewport.snapUp();

  viewport.modified = true;
}

void QOSPRayWindow::strafe(float du, float dv)
{
  ospcommon::affine3f xfm =
      ospcommon::affine3f::translate(dv * viewport.frame.l.vz)
    * ospcommon::affine3f::translate(-du * viewport.frame.l.vx);

  viewport.frame = xfm * viewport.frame;
  viewport.from = xfmPoint(xfm, viewport.from);
  viewport.at = xfmPoint(xfm, viewport.at);
  viewport.modified = true;
}

void QOSPRayWindow::dolly(float d)
{
  // camera distance from center point
  const float motionSpeed = 0.012f;

  float forward = d * motionSpeed * ospcommon::length(worldBounds.size());
  float oldDistance = length(viewport.at - viewport.from);
  float newDistance = oldDistance - forward;

  if(newDistance < 1e-3f)
    return;

  viewport.from = viewport.at - newDistance * viewport.frame.l.vy;
  viewport.frame.p = viewport.from;

  viewport.modified = true;
}

void QOSPRayWindow::stopRenderTimers()
{
  /* Pause continuous rendering during interaction and cancel any render restart
     timers. This keeps interaction more responsive (especially with low frame
     rates). */
  renderTimer.stop();
  renderRestartTimer.stop();
}

void QOSPRayWindow::startRenderTimers()
{
  // after a 0.5s delay, restart continuous rendering.
  renderRestartTimer.setSingleShot(true);
  renderRestartTimer.start(500);
}
