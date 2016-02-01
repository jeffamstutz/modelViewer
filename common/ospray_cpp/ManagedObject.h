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

#pragma once

#include <string>

#include <ospray.h>
#include <ospray/common/OSPCommon.h>

namespace ospray {
namespace cpp    {

//! \todo auto-commit mode

class ManagedObject
{
public:

  ManagedObject(OSPObject object = nullptr);
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

  // vec2
  void set(const std::string &name, const ospray::vec2i &v);
  void set(const std::string &name, const ospray::vec2f &v);

  // vec3
  void set(const std::string &name, const ospray::vec3i &v);
  void set(const std::string &name, const ospray::vec3f &v);

  // vec4
  void set(const std::string &name, const ospray::vec4f &v);

  // void*
  void set(const std::string &name, void *v);

  // OSPObject*
  void set(const std::string &name, OSPObject v);

  // osp::cpp::ManagedObject
  void set(const std::string &name, const ManagedObject &v);

  //! Commit to ospray
  void commit() const;

  //! Get the underlying OSPObject handle
  OSPObject handle() const;

protected:

  OSPObject m_object;
};

// Inlined function definitions ///////////////////////////////////////////////

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

inline void ManagedObject::set(const std::string &name, const vec2i &v)
{
  ospSetVec2i(m_object, name.c_str(), (const osp::vec2i&)v);
}

inline void ManagedObject::set(const std::string &name, const vec2f &v)
{
  ospSetVec2f(m_object, name.c_str(), (const osp::vec2f&)v);
}

inline void ManagedObject::set(const std::string &name, const vec3i &v)
{
  ospSetVec3i(m_object, name.c_str(), (const osp::vec3i&)v);
}

inline void ManagedObject::set(const std::string &name, const vec3f &v)
{
  ospSetVec3f(m_object, name.c_str(), (const osp::vec3f&)v);
}

inline void ManagedObject::set(const std::string &name, const vec4f &v)
{
  ospSetVec4f(m_object, name.c_str(), (const osp::vec4f&)v);
}

inline void ManagedObject::set(const std::string &name, void *v)
{
  ospSetVoidPtr(m_object, name.c_str(), v);
}

inline void ManagedObject::set(const std::string &name, OSPObject v)
{
  ospSetObject(m_object, name.c_str(), v);
}

inline void ManagedObject::set(const std::string &name, const ManagedObject &v)
{
  ospSetObject(m_object, name.c_str(), v.handle());
}

inline void ManagedObject::commit() const
{
  ospCommit(m_object);
}

inline OSPObject ManagedObject::handle() const
{
  return m_object;
}

}// namespace cpp
}// namespace ospray
