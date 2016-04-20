#include "CameraParser.h"

void CameraParser::parse(int ac, const char **&av)
{
  for (int i = 1; i < ac; i++) {
    const std::string arg = av[i];
    if (arg == "--camera" || arg == "-c") {
      m_cameraType = av[++i];
    }
  }

  finalize();
}

ospray::cpp::Camera CameraParser::camera()
{
  return m_camera;
}

void CameraParser::finalize()
{
  if (m_cameraType.empty())
    m_cameraType = "perspective";

  m_camera = ospray::cpp::Camera(m_cameraType.c_str());
  m_camera.set("pos", -1,  1, -1);
  m_camera.set("dir",  1, -1,  1);
  m_camera.commit();
}
