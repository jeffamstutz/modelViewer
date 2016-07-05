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

#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "widgets/QOSPRayWindow.h"

#include "common/commandline/LightsParser.h"
#include "common/commandline/SceneParser/MultiSceneParser.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_actionFileOpen_triggered()
{
  // Get a file from the user
  QFileDialog dialog;
  dialog.setFileMode(QFileDialog::ExistingFiles);

  if (!dialog.exec())
    return;// cancel button clicked...

  auto files = dialog.selectedFiles();

  Q_FOREACH (auto file, files) {
    try {
      auto data = openFile(file);
      addRenderSubWindow(data.first, data.second);
    } catch (const std::runtime_error &error) {
      QMessageBox msgBox;
      msgBox.setText(error.what());
      msgBox.exec();
      continue;
    }
  }
}

void MainWindow::on_actionTile_triggered()
{
  ui->mdiArea->setViewMode(QMdiArea::SubWindowView);
  ui->mdiArea->tileSubWindows();
}

void MainWindow::on_actionCascade_triggered()
{
  ui->mdiArea->setViewMode(QMdiArea::SubWindowView);
  ui->mdiArea->cascadeSubWindows();
}

void MainWindow::on_actionTabbed_triggered()
{
  ui->mdiArea->setViewMode(QMdiArea::TabbedView);
}

std::pair<OSPRenderer, ospcommon::box3f> MainWindow::openFile(QString fileName)
{
  // Create a basic renderer
  ospray::cpp::Renderer renderer("scivis");
  renderer.set("aoSamples", 1);
  renderer.set("shadowsEnabled", 1);

  // Fake a command line to reuse parser library
  auto file = fileName.toStdString();
  const char *commandline[2] = {nullptr, file.data()};
  const char **cl = (const char **)commandline;

  MultiSceneParser    msParser(renderer);
  DefaultLightsParser lParser(renderer);

  auto loadedScene = msParser.parse(2, cl);
  if (!loadedScene) {
    std::string msg = "ERROR: Failed to open file...\n\n";
    msg += file;
    throw std::runtime_error(msg);
  }

  lParser.parse(2, cl);

  auto model = msParser.model();
  renderer.set("model", model);
  renderer.commit();
  m_loadedModels.push_back({file.substr(file.find_last_of('/')+1), model});

  return {renderer.handle(), msParser.bbox()};
}

void MainWindow::addRenderSubWindow(OSPRenderer renderer,
                                    const ospcommon::box3f &bounds)
{
  // Create window in the MdiArea
  auto *window = new QOSPRayWindow(this, renderer, true);
  auto *subWindow = ui->mdiArea->addSubWindow(window);

  connect(window, SIGNAL(closing()), subWindow, SLOT(close()));

  subWindow->setMinimumSize(200, 200);

  if (ui->mdiArea->subWindowList().count() >= 1) {
    subWindow->show();
    on_actionTile_triggered();
  }
  else
    subWindow->showMaximized();

  window->setRenderingEnabled(true);
  window->setWorldBounds(bounds);
}

void MainWindow::on_actionAbout_triggered()
{
  QString msg = "OSPRay Debug Viewer\n\n";
  msg += "Source code at: http://github.com/jeffamstutz/ospDebugViewer";
  QMessageBox::about(this, "About", msg);
}
