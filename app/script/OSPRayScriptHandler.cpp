#include "OSPRayScriptHandler.h"

#include "chaiscript/chaiscript_stdlib.hpp"
#include "chaiscript/utility/utility.hpp"

using std::runtime_error;

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

#include <string>
using std::string;

#include <functional>

// Scripting callback functions ///////////////////////////////////////////////

namespace chaiospray {

void ospLoadModule(const string &name)
{
  ::ospLoadModule(name.c_str());
}

void ospSetString(OSPObject _object, const string &id, const string &s)
{
  ::ospSetString(_object, id.c_str(), s.c_str());
}

void ospSetObject(OSPObject _object, const string &id, OSPObject object)
{
  ::ospSetObject(_object, id.c_str(), object);
}

void ospSet1f(OSPObject _object, const string &id, float x)
{
  ::ospSet1f(_object, id.c_str(), x);
}

void ospSet1i(OSPObject _object, const string &id, int32 x)
{
  ::ospSet1i(_object, id.c_str(), x);
}

void ospSet2f(OSPObject _object, const string &id, float x, float y)
{
  ::ospSet2f(_object, id.c_str(), x, y);
}

void ospSet2i(OSPObject _object, const string &id, int x, int y)
{
  ::ospSet2i(_object, id.c_str(), x, y);
}

void ospSet3f(OSPObject _object, const string &id, float x, float y, float z)
{
  ::ospSet3f(_object, id.c_str(), x, y, z);
}

void ospSet3i(OSPObject _object, const string &id, int x, int y, int z)
{
  ::ospSet3i(_object, id.c_str(), x, y, z);
}

void ospSetVoidPtr(OSPObject _object, const string &id, void *v)
{
  ::ospSetVoidPtr(_object, id.c_str(), v);
}

void ospCommit(OSPObject object)
{
  ::ospCommit(object);
}

}

// OSPRayScriptHandler definitions ////////////////////////////////////////////

namespace ospray {

OSPRayScriptHandler::OSPRayScriptHandler(OSPModel    model,
                                         OSPRenderer renderer,
                                         OSPCamera   camera) :
  m_model(model),
  m_renderer(renderer),
  m_camera(camera),
  m_chai(chaiscript::Std_Lib::library()),
  m_running(false)
{
  registerScriptTypes();
  registerScriptFunctions();
}

OSPRayScriptHandler::~OSPRayScriptHandler()
{
  stop();
}

void OSPRayScriptHandler::runScriptFromFile(const std::string &file)
{
  if (m_running) {
    throw runtime_error("Cannot execute a script file when"
                        " running interactively!");
  }

  registerScriptObjects();

  try {
    m_chai.eval_file(file);
  } catch (const chaiscript::exception::eval_error &e) {
    cerr << "ERROR: script '" << file << "' executed with error(s):" << endl;
    cerr << "    " << e.what() << endl;
  }
}

void OSPRayScriptHandler::consoleLoop()
{
  registerScriptObjects();

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
  } while (m_running);

  cout << "**** EXIT COMMAND MODE *****" << endl;
}

void OSPRayScriptHandler::start()
{
  stop();
  cout << "**** START COMMAND MODE ****" << endl << endl;
  cout << "objects available:" << endl << endl;
  cout << "    c --> Camera"   << endl;
  cout << "    m --> Model"    << endl;
  cout << "    r --> Renderer" << endl;
  cout << endl;
  m_running = true;
  m_thread = std::thread(&OSPRayScriptHandler::consoleLoop, this);
}

void OSPRayScriptHandler::stop()
{
  m_running = false;
  if (m_thread.joinable())
    m_thread.join();
}

bool OSPRayScriptHandler::running()
{
  return m_running;
}

chaiscript::ChaiScript &OSPRayScriptHandler::scriptEngine()
{
  if (m_running)
    throw runtime_error("Cannot modify the script env while running!");

  return m_chai;
}

void OSPRayScriptHandler::registerScriptObjects()
{
  m_chai.add(chaiscript::var(m_model),    "m");
  m_chai.add(chaiscript::var(m_renderer), "r");
  m_chai.add(chaiscript::var(m_camera),   "c");
}

void OSPRayScriptHandler::registerScriptTypes()
{
  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());

  // Class types //

  chaiscript::utility::add_class<osp::cpp::ManagedObject>(*m, "ManagedObject",
     {
       chaiscript::constructor<osp::cpp::ManagedObject()>(),
       chaiscript::constructor<osp::cpp::ManagedObject(OSPObject)>()
     },
     {
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, const std::string &)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, int)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, int, int)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, int, int, int)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, float)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, float, float)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, float, float, float)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, double)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, double, double)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, double, double, double)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, void*)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, OSPObject)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, const osp::cpp::ManagedObject &)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(&osp::cpp::ManagedObject::commit), "commit"},
       {chaiscript::fun(&osp::cpp::ManagedObject::handle), "handle"}
     }
     );

  chaiscript::utility::add_class<osp::cpp::Camera>(*m, "Camera",
     {
       chaiscript::constructor<osp::cpp::Camera(const std::string &)>(),
       chaiscript::constructor<osp::cpp::Camera(const osp::cpp::Camera &)>(),
       chaiscript::constructor<osp::cpp::Camera(OSPCamera)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<osp::cpp::Data>(*m, "Data",
     {
       chaiscript::constructor<osp::cpp::Data(const osp::cpp::Data &)>(),
       chaiscript::constructor<osp::cpp::Data(OSPData)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<osp::cpp::FrameBuffer>(*m, "FrameBuffer",
     {
       chaiscript::constructor<osp::cpp::FrameBuffer(const osp::cpp::FrameBuffer &)>(),
       chaiscript::constructor<osp::cpp::FrameBuffer(OSPFrameBuffer)>()
     },
     {
       {chaiscript::fun(static_cast<void (osp::cpp::FrameBuffer::*)(osp::cpp::PixelOp &)>(&osp::cpp::FrameBuffer::setPixelOp)), "setPixelOp"},
       {chaiscript::fun(static_cast<void (osp::cpp::FrameBuffer::*)(OSPPixelOp)>(&osp::cpp::FrameBuffer::setPixelOp)), "setPixelOp"}
     }
     );

  chaiscript::utility::add_class<osp::cpp::Geometry>(*m, "Geometry",
     {
       chaiscript::constructor<osp::cpp::Geometry(const std::string &)>(),
       chaiscript::constructor<osp::cpp::Geometry(const osp::cpp::Geometry &)>(),
       chaiscript::constructor<osp::cpp::Geometry(OSPGeometry)>()
     },
     {
       {chaiscript::fun(static_cast<void (osp::cpp::Geometry::*)(osp::cpp::Material &)>(&osp::cpp::Geometry::setMaterial)), "setMaterial"},
       {chaiscript::fun(static_cast<void (osp::cpp::Geometry::*)(OSPMaterial)>(&osp::cpp::Geometry::setMaterial)), "setMaterial"}
     }
     );

  chaiscript::utility::add_class<osp::cpp::Light>(*m, "Light",
     {
       chaiscript::constructor<osp::cpp::Light(const osp::cpp::Light &)>(),
       chaiscript::constructor<osp::cpp::Light(OSPLight)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<osp::cpp::Material>(*m, "Material",
     {
       chaiscript::constructor<osp::cpp::Material(const osp::cpp::Material &)>(),
       chaiscript::constructor<osp::cpp::Material(OSPMaterial)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<osp::cpp::Model>(*m, "Model",
     {
       chaiscript::constructor<osp::cpp::Model()>(),
       chaiscript::constructor<osp::cpp::Model(const osp::cpp::Model &)>(),
       chaiscript::constructor<osp::cpp::Model(OSPModel)>()
     },
     {
       {chaiscript::fun(static_cast<void (osp::cpp::Model::*)(osp::cpp::Geometry &)>(&osp::cpp::Model::addGeometry)), "addGeometry"},
       {chaiscript::fun(static_cast<void (osp::cpp::Model::*)(OSPGeometry)>(&osp::cpp::Model::addGeometry)), "addGeometry"},
       {chaiscript::fun(static_cast<void (osp::cpp::Model::*)(osp::cpp::Geometry &)>(&osp::cpp::Model::removeGeometry)), "removeGeometry"},
       {chaiscript::fun(static_cast<void (osp::cpp::Model::*)(OSPGeometry)>(&osp::cpp::Model::removeGeometry)), "removeGeometry"},
       {chaiscript::fun(static_cast<void (osp::cpp::Model::*)(osp::cpp::Volume &)>(&osp::cpp::Model::addVolume)), "addVolume"},
       {chaiscript::fun(static_cast<void (osp::cpp::Model::*)(OSPVolume)>(&osp::cpp::Model::addVolume)), "addVolume"}
     }
     );

  chaiscript::utility::add_class<osp::cpp::PixelOp>(*m, "PixelOp",
     {
       chaiscript::constructor<osp::cpp::PixelOp(const std::string &)>(),
       chaiscript::constructor<osp::cpp::PixelOp(const osp::cpp::PixelOp &)>(),
       chaiscript::constructor<osp::cpp::PixelOp(OSPPixelOp)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<osp::cpp::Renderer>(*m, "Renderer",
     {
       chaiscript::constructor<osp::cpp::Renderer(const std::string &)>(),
       chaiscript::constructor<osp::cpp::Renderer(const osp::cpp::Renderer &)>(),
       chaiscript::constructor<osp::cpp::Renderer(OSPRenderer)>()
     },
     {
       {chaiscript::fun(&osp::cpp::Renderer::newMaterial), "newMaterial"},
       {chaiscript::fun(&osp::cpp::Renderer::newLight), "newLight"}
     }
     );

  chaiscript::utility::add_class<osp::cpp::TransferFunction>(*m, "TransferFunction",
     {
       chaiscript::constructor<osp::cpp::TransferFunction(const std::string &)>(),
       chaiscript::constructor<osp::cpp::TransferFunction(const osp::cpp::TransferFunction &)>(),
       chaiscript::constructor<osp::cpp::TransferFunction(OSPTransferFunction)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<osp::cpp::Texture2D>(*m, "Texture2D",
     {
       chaiscript::constructor<osp::cpp::Texture2D(const osp::cpp::Texture2D &)>(),
       chaiscript::constructor<osp::cpp::Texture2D(OSPTexture2D)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<osp::cpp::Volume>(*m, "Volume",
     {
       chaiscript::constructor<osp::cpp::Volume(const std::string &)>(),
       chaiscript::constructor<osp::cpp::Volume(const osp::cpp::Volume &)>(),
       chaiscript::constructor<osp::cpp::Volume(OSPVolume)>()
     },
     {
     }
     );

  m_chai.add(m);

  // Inheritance relationships //

  // osp objects
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Camera>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Data>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::FrameBuffer>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Geometry>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Light>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Material>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Model>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::PixelOp>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Renderer>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject,
                                    osp::TransferFunction>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Texture2D>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Volume>());

  // osp::cpp objects
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Camera>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Data>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::FrameBuffer>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Geometry>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Light>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Material>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Model>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::PixelOp>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Renderer>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::TransferFunction>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Texture2D>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Volume>());
}

void OSPRayScriptHandler::registerScriptFunctions()
{
  m_chai.add(chaiscript::fun(&chaiospray::ospLoadModule), "ospLoadModule");
  m_chai.add(chaiscript::fun(&chaiospray::ospSetString),  "ospSetString" );
  m_chai.add(chaiscript::fun(&chaiospray::ospSetObject),  "ospSetObject" );
  m_chai.add(chaiscript::fun(&chaiospray::ospSet1f),      "ospSet1f"     );
  m_chai.add(chaiscript::fun(&chaiospray::ospSet1i),      "ospSet1i"     );
  m_chai.add(chaiscript::fun(&chaiospray::ospSet2f),      "ospSet2f"     );
  m_chai.add(chaiscript::fun(&chaiospray::ospSet2i),      "ospSet2i"     );
  m_chai.add(chaiscript::fun(&chaiospray::ospSet3f),      "ospSet3f"     );
  m_chai.add(chaiscript::fun(&chaiospray::ospSet3i),      "ospSet3i"     );
  m_chai.add(chaiscript::fun(&chaiospray::ospSetVoidPtr), "ospSetVoidPtr");
  m_chai.add(chaiscript::fun(&chaiospray::ospCommit),     "ospCommit"    );
}

}// namespace ospray
