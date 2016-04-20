#include "RendererParser.h"

void RendererParser::parse(int ac, const char **&av)
{
  for (int i = 1; i < ac; i++) {
    const std::string arg = av[i];
    if (arg == "--renderer" || arg == "-r") {
      assert(i+1 < ac);
      m_rendererType = av[++i];
    }
  }

  finalize();
}

ospray::cpp::Renderer RendererParser::renderer()
{
  return m_renderer;
}

void RendererParser::finalize()
{
  if (m_rendererType.empty())
    m_rendererType = "ao1";

  m_renderer = ospray::cpp::Renderer(m_rendererType.c_str());
  m_renderer.commit();
}
