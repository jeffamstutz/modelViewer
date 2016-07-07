#include "OSPRayScriptHandler.h"

#include "chaiscript/chaiscript_stdlib.hpp"
#include "chaiscript/utility/utility.hpp"
#include "OSPRayChaiUtil.h"

using std::runtime_error;

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

#include <string>
using std::string;
using std::stringstream;

#include <functional>

#ifdef USE_SYSTEM_READLINE
#  include <readline/readline.h>
#  include <readline/history.h>
#else

static char *mystrdup (const char *s) {
  size_t len = strlen(s); // Space for length plus nul
  char *d = static_cast<char*>(malloc (len+1));
  if (d == nullptr) return nullptr;          // No memory
#if 0//MSVC
  strcpy_s(d, len+1, s);                        // Copy the characters
#else
  strncpy(d,s,len);                        // Copy the characters
#endif
  d[len] = '\0';
  return d;                            // Return the new string
}

static char* readline(const char* p)
{
  string retval;
  cout << p ;
  getline(std::cin, retval);
  return cin.eof() ? nullptr : mystrdup(retval.c_str());
}


static void add_history(const char*){}
static void using_history(){}
#endif

// Static helper functions ////////////////////////////////////////////////////

static std::string get_next_command() {
  std::string retval("quit");
  if ( ! std::cin.eof() ) {
    char *input_raw = readline("% ");
    if ( input_raw ) {
      add_history(input_raw);

      std::string val(input_raw);
      size_t pos = val.find_first_not_of("\t \n");
      if (pos != std::string::npos)
      {
        val.erase(0, pos);
      }
      pos = val.find_last_not_of("\t \n");
      if (pos != std::string::npos)
      {
        val.erase(pos+1, std::string::npos);
      }

      retval = val;

      ::free(input_raw);
    }
  }
  return retval;
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
  script::registerScriptTypes(m_chai);
  script::registerScriptFunctions(m_chai);

  std::stringstream ss;
  ss << "Commands available:" << endl << endl;
  ss << "exit       --> exit command mode" << endl;
  ss << "done       --> synonomous with 'exit'" << endl;
  ss << "quit       --> synonomous with 'exit'" << endl;
  ss << "run [file] --> execute a script file" << endl << endl;

  ss << "OSPRay viewer objects available:" << endl << endl;
  ss << "c --> Camera"   << endl;
  ss << "m --> Model"    << endl;
  ss << "r --> Renderer" << endl << endl;

  ss << "OSPRay API functions available:" << endl << endl;
  ss << "ospLoadModule(module_name)"                << endl;
  ss << "ospSetString(object, id, string)"          << endl;
  ss << "ospSetObject(object, id, object)"          << endl;
  ss << "ospSet1f(object, id, float)"               << endl;
  ss << "ospSet2f(object, id, float, float)"        << endl;
  ss << "ospSet3f(object, id, float, float, float)" << endl;
  ss << "ospSet1i(object, id, int)"                 << endl;
  ss << "ospSet2i(object, id, int, int)"            << endl;
  ss << "ospSet3i(object, id, int, int, int)"       << endl;
  ss << "ospSetVoidPtr(object, id, ptr)"            << endl;
  ss << "ospCommit(object)"                         << endl;
  ss << endl;

  m_helpText = ss.str();
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

  script::registerScriptObjects(m_chai, m_model, m_renderer, m_camera);

  try {
    m_chai.eval_file(file);
  } catch (const chaiscript::exception::eval_error &e) {
    cerr << "ERROR: script '" << file << "' executed with error(s):" << endl;
    cerr << "    " << e.what() << endl;
  }
}

void OSPRayScriptHandler::consoleLoop()
{
  script::registerScriptObjects(m_chai, m_model, m_renderer, m_camera);

  using_history();

  do {
    std::string input = get_next_command();

    if (input == "done" || input == "exit" || input == "quit") {
      break;
    } else if (input == "help") {
      cout << m_helpText << endl;
      continue;
    } else {
      stringstream ss(input);
      string command, arg;
      ss >> command >> arg;
      if (command == "run") {
        runChaiFile(arg);
        continue;
      }
    }

    runChaiLine(input);

  } while (m_running);

  cout << "**** EXIT COMMAND MODE *****" << endl;
}

void OSPRayScriptHandler::runChaiLine(const std::string &line)
{
  try {
    m_chai.eval(line);
  } catch (const chaiscript::exception::eval_error &e) {
    cerr << e.what() << endl;
  }
}

void OSPRayScriptHandler::runChaiFile(const std::string &file)
{
  try {
    m_chai.eval_file(file);
  } catch (const std::runtime_error &e) {
    cerr << e.what() << endl;
  }
}

void OSPRayScriptHandler::start()
{
  stop();
  cout << "**** START COMMAND MODE ****" << endl << endl;
  cout << "Type 'help' to see available objects and functions." << endl;
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


}// namespace ospray
