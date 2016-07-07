
#include "OSPRayChaiUtil.h"

#include "chaiscript/utility/utility.hpp"

#include <ospray_cpp/Data.h>
#include <ospray_cpp/FrameBuffer.h>
#include <ospray_cpp/Geometry.h>
#include <ospray_cpp/Light.h>
#include <ospray_cpp/Material.h>
#include <ospray_cpp/PixelOp.h>
#include <ospray_cpp/Texture2D.h>
#include <ospray_cpp/TransferFunction.h>
#include <ospray_cpp/Volume.h>

using chaiscript::ChaiScript;
using std::string;

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

void ospSet1i(OSPObject _object, const string &id, int x)
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

}// namespace chaiospray

namespace ospray {
namespace script {

// OSPRayChaiUtil definitions /////////////////////////////////////////////////

void registerScriptObjects(ChaiScript &chai,
                           ospray::cpp::Model model,
                           ospray::cpp::Renderer renderer,
                           ospray::cpp::Camera camera)
{
  chai.add(chaiscript::var(model),    "m");
  chai.add(chaiscript::var(renderer), "r");
  chai.add(chaiscript::var(camera),   "c");
}

void registerScriptTypes(ChaiScript &chai)
{
  chaiscript::ModulePtr m = chaiscript::ModulePtr(new chaiscript::Module());

  // Class types //

  chaiscript::utility::add_class<ospray::cpp::ManagedObject_T<>>(*m, "ManagedObject",
     {
       chaiscript::constructor<ospray::cpp::ManagedObject_T<>()>(),
       chaiscript::constructor<ospray::cpp::ManagedObject_T<>(OSPObject)>()
     },
     {
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, const std::string &)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, int)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, int, int)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, int, int, int)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, float)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, float, float)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, float, float, float)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, double)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, double, double)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, double, double, double)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, void*)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, OSPObject)>(&ospray::cpp::ManagedObject::set)), "set"},
       //{chaiscript::fun(static_cast<void (ospray::cpp::ManagedObject::*)(const std::string &, const ospray::cpp::ManagedObject &)>(&ospray::cpp::ManagedObject::set)), "set"},
       {chaiscript::fun(&ospray::cpp::ManagedObject::commit), "commit"},
       {chaiscript::fun(&ospray::cpp::ManagedObject::object), "handle"}
     }
     );

  chaiscript::utility::add_class<ospray::cpp::Camera>(*m, "Camera",
     {
       chaiscript::constructor<ospray::cpp::Camera(const std::string &)>(),
       chaiscript::constructor<ospray::cpp::Camera(const ospray::cpp::Camera &)>(),
       chaiscript::constructor<ospray::cpp::Camera(OSPCamera)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<ospray::cpp::Data>(*m, "Data",
     {
       chaiscript::constructor<ospray::cpp::Data(const ospray::cpp::Data &)>(),
       chaiscript::constructor<ospray::cpp::Data(OSPData)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<ospray::cpp::FrameBuffer>(*m, "FrameBuffer",
     {
       chaiscript::constructor<ospray::cpp::FrameBuffer(const ospray::cpp::FrameBuffer &)>(),
       chaiscript::constructor<ospray::cpp::FrameBuffer(OSPFrameBuffer)>()
     },
     {
       {chaiscript::fun(static_cast<void (ospray::cpp::FrameBuffer::*)(ospray::cpp::PixelOp &)>(&ospray::cpp::FrameBuffer::setPixelOp)), "setPixelOp"},
       {chaiscript::fun(static_cast<void (ospray::cpp::FrameBuffer::*)(OSPPixelOp)>(&ospray::cpp::FrameBuffer::setPixelOp)), "setPixelOp"}
     }
     );

  chaiscript::utility::add_class<ospray::cpp::Geometry>(*m, "Geometry",
     {
       chaiscript::constructor<ospray::cpp::Geometry(const std::string &)>(),
       chaiscript::constructor<ospray::cpp::Geometry(const ospray::cpp::Geometry &)>(),
       chaiscript::constructor<ospray::cpp::Geometry(OSPGeometry)>()
     },
     {
       {chaiscript::fun(static_cast<void (ospray::cpp::Geometry::*)(ospray::cpp::Material &)>(&ospray::cpp::Geometry::setMaterial)), "setMaterial"},
       {chaiscript::fun(static_cast<void (ospray::cpp::Geometry::*)(OSPMaterial)>(&ospray::cpp::Geometry::setMaterial)), "setMaterial"}
     }
     );

  chaiscript::utility::add_class<ospray::cpp::Light>(*m, "Light",
     {
       chaiscript::constructor<ospray::cpp::Light(const ospray::cpp::Light &)>(),
       chaiscript::constructor<ospray::cpp::Light(OSPLight)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<ospray::cpp::Material>(*m, "Material",
     {
       chaiscript::constructor<ospray::cpp::Material(const ospray::cpp::Material &)>(),
       chaiscript::constructor<ospray::cpp::Material(OSPMaterial)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<ospray::cpp::Model>(*m, "Model",
     {
       chaiscript::constructor<ospray::cpp::Model()>(),
       chaiscript::constructor<ospray::cpp::Model(const ospray::cpp::Model &)>(),
       chaiscript::constructor<ospray::cpp::Model(OSPModel)>()
     },
     {
       {chaiscript::fun(static_cast<void (ospray::cpp::Model::*)(ospray::cpp::Geometry &)>(&ospray::cpp::Model::addGeometry)), "addGeometry"},
       {chaiscript::fun(static_cast<void (ospray::cpp::Model::*)(OSPGeometry)>(&ospray::cpp::Model::addGeometry)), "addGeometry"},
       {chaiscript::fun(static_cast<void (ospray::cpp::Model::*)(ospray::cpp::Geometry &)>(&ospray::cpp::Model::removeGeometry)), "removeGeometry"},
       {chaiscript::fun(static_cast<void (ospray::cpp::Model::*)(OSPGeometry)>(&ospray::cpp::Model::removeGeometry)), "removeGeometry"},
       {chaiscript::fun(static_cast<void (ospray::cpp::Model::*)(ospray::cpp::Volume &)>(&ospray::cpp::Model::addVolume)), "addVolume"},
       {chaiscript::fun(static_cast<void (ospray::cpp::Model::*)(OSPVolume)>(&ospray::cpp::Model::addVolume)), "addVolume"}
     }
     );

  chaiscript::utility::add_class<ospray::cpp::PixelOp>(*m, "PixelOp",
     {
       chaiscript::constructor<ospray::cpp::PixelOp(const std::string &)>(),
       chaiscript::constructor<ospray::cpp::PixelOp(const ospray::cpp::PixelOp &)>(),
       chaiscript::constructor<ospray::cpp::PixelOp(OSPPixelOp)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<ospray::cpp::Renderer>(*m, "Renderer",
     {
       chaiscript::constructor<ospray::cpp::Renderer(const std::string &)>(),
       chaiscript::constructor<ospray::cpp::Renderer(const ospray::cpp::Renderer &)>(),
       chaiscript::constructor<ospray::cpp::Renderer(OSPRenderer)>()
     },
     {
       {chaiscript::fun(&ospray::cpp::Renderer::newMaterial), "newMaterial"},
       {chaiscript::fun(&ospray::cpp::Renderer::newLight), "newLight"}
     }
     );

  chaiscript::utility::add_class<ospray::cpp::TransferFunction>(*m, "TransferFunction",
     {
       chaiscript::constructor<ospray::cpp::TransferFunction(const std::string &)>(),
       chaiscript::constructor<ospray::cpp::TransferFunction(const ospray::cpp::TransferFunction &)>(),
       chaiscript::constructor<ospray::cpp::TransferFunction(OSPTransferFunction)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<ospray::cpp::Texture2D>(*m, "Texture2D",
     {
       chaiscript::constructor<ospray::cpp::Texture2D(const ospray::cpp::Texture2D &)>(),
       chaiscript::constructor<ospray::cpp::Texture2D(OSPTexture2D)>()
     },
     {
     }
     );

  chaiscript::utility::add_class<ospray::cpp::Volume>(*m, "Volume",
     {
       chaiscript::constructor<ospray::cpp::Volume(const std::string &)>(),
       chaiscript::constructor<ospray::cpp::Volume(const ospray::cpp::Volume &)>(),
       chaiscript::constructor<ospray::cpp::Volume(OSPVolume)>()
     },
     {
     }
     );

  chai.add(m);

  // Inheritance relationships //

  // osp objects
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::Camera>());
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::Data>());
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::FrameBuffer>());
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::Geometry>());
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::Light>());
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::Material>());
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::Model>());
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::PixelOp>());
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::Renderer>());
  chai.add(chaiscript::base_class<osp::ManagedObject,
                                  osp::TransferFunction>());
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::Texture2D>());
  chai.add(chaiscript::base_class<osp::ManagedObject, osp::Volume>());

  // ospray::cpp objects
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::Camera>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::Data>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::FrameBuffer>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::Geometry>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::Light>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::Material>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::Model>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::PixelOp>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::Renderer>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::TransferFunction>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                  ospray::cpp::Texture2D>());
  chai.add(chaiscript::base_class<ospray::cpp::ManagedObject,
                                    ospray::cpp::Volume>());
}

void registerScriptFunctions(ChaiScript &chai)
{
  chai.add(chaiscript::fun(&chaiospray::ospLoadModule), "ospLoadModule");
  chai.add(chaiscript::fun(&chaiospray::ospSetString),  "ospSetString" );
  chai.add(chaiscript::fun(&chaiospray::ospSetObject),  "ospSetObject" );
  chai.add(chaiscript::fun(&chaiospray::ospSet1f),      "ospSet1f"     );
  chai.add(chaiscript::fun(&chaiospray::ospSet1i),      "ospSet1i"     );
  chai.add(chaiscript::fun(&chaiospray::ospSet2f),      "ospSet2f"     );
  chai.add(chaiscript::fun(&chaiospray::ospSet2i),      "ospSet2i"     );
  chai.add(chaiscript::fun(&chaiospray::ospSet3f),      "ospSet3f"     );
  chai.add(chaiscript::fun(&chaiospray::ospSet3i),      "ospSet3i"     );
  chai.add(chaiscript::fun(&chaiospray::ospSetVoidPtr), "ospSetVoidPtr");
  chai.add(chaiscript::fun(&chaiospray::ospCommit),     "ospCommit"    );
}

}// namespace script
}// namespace ospray
