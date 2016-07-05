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

#pragma once

#include "Viewport.h"
// ospray wrappers
#include <ospray_cpp/Camera.h>
#include <ospray_cpp/FrameBuffer.h>
#include <ospray_cpp/Renderer.h>
// qt
#include <QtGui>
#include <QGLWidget>

class QOSPRayWindow : public QGLWidget
{
  Q_OBJECT

public:

  QOSPRayWindow(QMainWindow *parent,
                ospray::cpp::Renderer renderer,
                bool showFrameRate);
  virtual ~QOSPRayWindow();

  void setRenderingEnabled(bool renderingEnabled);
  void setRotationRate(float rotationRate);
  void setWorldBounds(const ospcommon::box3f &worldBounds);

signals:

  void closing();

private slots:

  void showContextMenu(const QPoint &);

private:

  void resetAccumulationBuffer();

  void paintGL() override;
  void resizeGL(int width, int height) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void closeEvent(QCloseEvent *) override;

  /*! rotate about center point */
  void rotateCenter(float du, float dv);

  /*! strafe the camera from / at point */
  void strafe(float du, float dv);

  void dolly(float d);

  void stopRenderTimers();
  void startRenderTimers();

  // Private data /////////////////////////////////////////////////////////////

  /*! Parent Qt window. */
  QMainWindow *parent;

  /*! Display the frame rate in the main window title bar. */
  bool showFrameRate;

  /*! only render when this flag is true. this allows the window to be created
   *  before all required components are ospCommit()'d. */
  bool renderingEnabled;

  /*! rotation rate to automatically rotate view. */
  float rotationRate;

  /*! timer used to trigger continuous re-renders (for progressive refinement,
   *  automatic rotation, etc.). */
  QTimer renderTimer;

  /*! timer used to restart continuous rendering after a delay during
   *  interaction. */
  QTimer renderRestartTimer;

  /*! Timer to measure elapsed time over a single frame. */
  QTime renderFrameTimer;

  ospcommon::vec2i windowSize;
  Viewport viewport;
  ospcommon::box3f worldBounds;
  QPoint lastMousePosition;

  ospray::cpp::FrameBuffer frameBuffer;
  ospray::cpp::Renderer renderer;
  ospray::cpp::Camera camera;
};
