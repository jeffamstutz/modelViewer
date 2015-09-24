#include "OSPRayScriptHandler.h"

#include <chaiscript/utility/utility.hpp>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

#include <string>
using std::string;

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

class testClass
{
public:
  void sayHello() { cout << "HELLO!!" << endl; }
  void sayHello(int x) { cout << "HELLO with x!!..." << x << endl; }
};

namespace ospray {

OSPRayScriptHandler::OSPRayScriptHandler(OSPModel    model,
                                         OSPRenderer renderer,
                                         OSPCamera   camera) :
  m_model(model),
  m_renderer(renderer),
  m_camera(camera),
  m_running(false)
{
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

void OSPRayScriptHandler::registerScriptObjects()
{
  m_chai.add(chaiscript::var((osp::ManagedObject*)m_model),    "model"   );
  m_chai.add(chaiscript::var((osp::ManagedObject*)m_renderer), "renderer");
  m_chai.add(chaiscript::var((osp::ManagedObject*)m_camera),   "camera"  );
}

void OSPRayScriptHandler::registerScriptFunctions()
{
  m_chai.add(chaiscript::fun(&chaiospray::ospSet1f),  "ospSet1f");
  m_chai.add(chaiscript::fun(&chaiospray::ospSet1i),  "ospSet1i");
  m_chai.add(chaiscript::fun(&chaiospray::ospSet2f),  "ospSet2f");
  m_chai.add(chaiscript::fun(&chaiospray::ospSet2i),  "ospSet2i");
  m_chai.add(chaiscript::fun(&chaiospray::ospSet3f),  "ospSet3f");
  m_chai.add(chaiscript::fun(&chaiospray::ospSet3i),  "ospSet3i");
  m_chai.add(chaiscript::fun(&chaiospray::ospCommit), "ospCommit");

  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());
  chaiscript::utility::add_class<testClass>(*m, "testClass",
     {chaiscript::constructor<testClass()>()},
     {
       {chaiscript::fun(static_cast<void (testClass::*)()>(&testClass::sayHello)), "sayHello"},
       {chaiscript::fun(static_cast<void (testClass::*)(int)>(&testClass::sayHello)), "sayHello"}
     }
     );

  m_chai.add(m);
}

}// namespace ospray
