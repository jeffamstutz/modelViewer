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
  // Create a basic AO renderer
  ospray::cpp::Renderer renderer("ao1");

  // Get a file from the user
  QFileDialog dialog;
  dialog.exec();
  dialog.setFileMode(QFileDialog::ExistingFile);
  auto file = dialog.selectedFiles()[0].toStdString();

  // Fake a command line to reuse parser library
  const char *commandline[2] = {nullptr, file.data()};
  const char ** cl = (const char **)commandline;

  MultiSceneParser parser(renderer);
  parser.parse(2, cl);

  renderer.set("model", parser.model());

  // Create window in the MdiArea
  auto *window = new QOSPRayWindow(this, renderer.handle(), true);
  auto *subWindow = ui->mdiArea->addSubWindow(window);

  //subWindow->setWindowTitle("OSPRay Window");
  subWindow->setMinimumSize(200, 200);

  if (ui->mdiArea->subWindowList().count() >= 1) {
    subWindow->show();
    on_actionTile_triggered();
  }
  else
    subWindow->showMaximized();

  window->setRenderingEnabled(true);
  window->setWorldBounds(parser.bbox());
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
