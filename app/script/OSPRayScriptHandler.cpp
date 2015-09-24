#include "OSPRayScriptHandler.h"

#include "chaiscript/chaiscript_stdlib.hpp"
#include "chaiscript/utility/utility.hpp"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

#include <string>
using std::string;

#include <functional>

// Scripting callback functions ///////////////////////////////////////////////

namespace chaiospray {

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
  cout << "objects available: 'renderer', 'camera', 'model'" << endl << endl;
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

void OSPRayScriptHandler::registerScriptObjects()
{
  m_chai.add(chaiscript::var(m_model),    "model"   );
  m_chai.add(chaiscript::var(m_renderer), "renderer");
  m_chai.add(chaiscript::var(m_camera),   "camera"  );
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
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, void*)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (osp::cpp::ManagedObject::*)(const std::string &, OSPObject)>(&osp::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(&osp::cpp::ManagedObject::commit), "commit"},
       {chaiscript::fun(&osp::cpp::ManagedObject::handle), "handle"}
     }
     );

  chaiscript::utility::add_class<osp::cpp::Renderer>(*m, "Renderer",
     {
       chaiscript::constructor<osp::cpp::Renderer(const std::string &)>(),
       chaiscript::constructor<osp::cpp::Renderer(const osp::cpp::Renderer &)>(),
       chaiscript::constructor<osp::cpp::Renderer(OSPRenderer)>()
     },
     {
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

  chaiscript::utility::add_class<osp::cpp::Model>(*m, "Model",
     {
       chaiscript::constructor<osp::cpp::Model()>(),
       chaiscript::constructor<osp::cpp::Model(const osp::cpp::Model &)>(),
       chaiscript::constructor<osp::cpp::Model(OSPModel)>()
     },
     {
     }
     );

  m_chai.add(m);

  // Inheritance relationships //

  // osp objects
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Camera>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Model>());
  m_chai.add(chaiscript::base_class<osp::ManagedObject, osp::Renderer>());

  // osp::cpp objects
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Camera>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Model>());
  m_chai.add(chaiscript::base_class<osp::cpp::ManagedObject,
                                    osp::cpp::Renderer>());
}

void OSPRayScriptHandler::registerScriptFunctions()
{
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
