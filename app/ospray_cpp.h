// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
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

#include <ospray/ospray.h>

namespace osp {
namespace cpp {

//! \todo auto-commit mode

class ManagedObject
{
public:

  ManagedObject();
  ManagedObject(OSPObject object);
  virtual ~ManagedObject();

  // Parameter 'set' functions //

  // string
  void set(const std::string &name, const std::string &v);

  // int
  void set(const std::string &name, int v);
  void set(const std::string &name, int v1, int v2);
  void set(const std::string &name, int v1, int v2, int v3);

  // float
  void set(const std::string &name, float v);
  void set(const std::string &name, float v1, float v2);
  void set(const std::string &name, float v1, float v2, float v3);

  // double
  void set(const std::string &name, double v);
  void set(const std::string &name, double v1, double v2);
  void set(const std::string &name, double v1, double v2, double v3);

  // void*
  void set(const std::string &name, void *v);

  // OSPObject*
  void set(const std::string &name, OSPObject v);

  //! \todo add get functions

  //! Commit to ospray
  void commit() const;

  //! Get the underlying OSPObject handle
  OSPObject handle() const;

protected:

  OSPObject m_object;
};

// Inlined function definitions ///////////////////////////////////////////////

inline ManagedObject::ManagedObject() :
  m_object(NULL)
{
}

inline ManagedObject::ManagedObject(OSPObject object) :
  m_object(object)
{
}

inline ManagedObject::~ManagedObject()
{
}

inline void ManagedObject::set(const std::string &name, const std::string &v)
{
  ospSetString(m_object, name.c_str(), v.c_str());
}

inline void ManagedObject::set(const std::string &name, int v)
{
  ospSet1i(m_object, name.c_str(), v);
}

inline void ManagedObject::set(const std::string &name, int v1, int v2)
{
  ospSet2i(m_object, name.c_str(), v1, v2);
}

inline void ManagedObject::set(const std::string &name, int v1, int v2, int v3)
{
  ospSet3i(m_object, name.c_str(), v1, v2, v3);
}

inline void ManagedObject::set(const std::string &name, float v)
{
  ospSet1f(m_object, name.c_str(), v);
}

inline void ManagedObject::set(const std::string &name, float v1, float v2)
{
  ospSet2f(m_object, name.c_str(), v1, v2);
}

inline void ManagedObject::set(const std::string &name,
                               float v1, float v2, float v3)
{
  ospSet3f(m_object, name.c_str(), v1, v2, v3);
}

inline void ManagedObject::set(const std::string &name, double v)
{
  ospSet1f(m_object, name.c_str(), v);
}

inline void ManagedObject::set(const std::string &name, double v1, double v2)
{
  ospSet2f(m_object, name.c_str(), v1, v2);
}

inline void ManagedObject::set(const std::string &name,
                               double v1, double v2, double v3)
{
  ospSet3f(m_object, name.c_str(), v1, v2, v3);
}

inline void ManagedObject::set(const std::string &name, void *v)
{
  ospSetVoidPtr(m_object, name.c_str(), v);
}

inline void ManagedObject::set(const std::string &name, OSPObject v)
{
  ospSetObject(m_object, name.c_str(), v);
}

inline void ManagedObject::commit() const
{
  ospCommit(m_object);
}

inline OSPObject ManagedObject::handle() const
{
  return m_object;
}

// Camera class ///////////////////////////////////////////////////////////////

class Camera : public ManagedObject
{
public:

  Camera(const std::string &type);
  Camera(const Camera &copy);
  Camera(OSPCamera existing);
};

// Camera inlined members //

inline Camera::Camera(const std::string &type)
{
  OSPCamera c = ospNewCamera(type.c_str());
  if (c) {
    m_object = c;
  } else {
    throw std::runtime_error("Failed to create OSPCamera!");
  }
}

inline Camera::Camera(const Camera &copy) :
  ManagedObject(copy.handle())
{
}

inline Camera::Camera(OSPCamera existing) :
  ManagedObject(existing)
{
}

// Model class ////////////////////////////////////////////////////////////////

class Model : public ManagedObject
{
public:

  Model();
  Model(const Model &copy);
  Model(OSPModel existing);
};

// Model inlined members //

inline Model::Model()
{
  OSPModel c = ospNewModel();
  if (c) {
    m_object = c;
  } else {
    throw std::runtime_error("Failed to create OSPModel!");
  }
}

inline Model::Model(const Model &copy) :
  ManagedObject(copy.handle())
{
}

inline Model::Model(OSPModel existing) :
  ManagedObject(existing)
{
}

// Renderer class /////////////////////////////////////////////////////////////

class Renderer : public ManagedObject
{
public:

  Renderer(const std::string &type);
  Renderer(const Renderer &copy);
  Renderer(OSPRenderer existing);
};

// Renderer inlined members //

inline Renderer::Renderer(const std::string &type)
{
  OSPRenderer c = ospNewRenderer(type.c_str());
  if (c) {
    m_object = c;
  } else {
    throw std::runtime_error("Failed to create OSPRenderer!");
  }
}

inline Renderer::Renderer(const Renderer &copy) :
  ManagedObject(copy.handle())
{
}

inline Renderer::Renderer(OSPRenderer existing) :
  ManagedObject(existing)
{
}

}// namespace cpp
}// namespace osp
