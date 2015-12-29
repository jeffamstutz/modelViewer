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

#include <ospray_cpp/ManagedObject.h>
#include <ospray_cpp/PixelOp.h>

namespace ospray {
namespace cpp    {

class FrameBuffer : public ManagedObject
{
public:

  FrameBuffer(const FrameBuffer &copy);
  FrameBuffer(OSPFrameBuffer existing);

  void setPixelOp(PixelOp &p);
  void setPixelOp(OSPPixelOp p);
};

// Inlined function definitions ///////////////////////////////////////////////

inline FrameBuffer::FrameBuffer(const FrameBuffer &copy) :
  ManagedObject(copy.handle())
{
}

inline FrameBuffer::FrameBuffer(OSPFrameBuffer existing) :
  ManagedObject(existing)
{
}

inline void FrameBuffer::setPixelOp(PixelOp &p)
{
  setPixelOp((OSPPixelOp)p.handle());
}

inline void FrameBuffer::setPixelOp(OSPPixelOp p)
{
  ospSetPixelOp((OSPFrameBuffer)handle(), p);
}

}// namespace cpp
}// namespace ospray
