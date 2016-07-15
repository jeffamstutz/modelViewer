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

#include <ostream>
#include <ospcommon/AffineSpace.h>

struct Viewport
{
  Viewport();

  ospcommon::vec3f from {0, -1, 0};
  ospcommon::vec3f at {0, 0, 0};
  ospcommon::vec3f up {0, 1, 0};

  /*! aspect ratio (width / height) */
  float aspect {1.f};

  /*! vertical field of view (degrees) */
  float fovY {60.f};

  /*! this flag should be set every time the viewport is modified */
  bool modified {true};

  /*! camera frame in which the Y axis is the depth axis, and X
    and Z axes are parallel to the screen X and Y axis. The frame
    itself remains normalized. */
  ospcommon::affine3f frame;

  /*! set up vector */
  void setUp(const ospcommon::vec3f &vec);

  /*! set frame 'up' vector. if this vector is (0,0,0) the window will
   *not* apply the up-vector after camera manipulation */
  void snapUp();
};

// Inline Viewport definitions ////////////////////////////////////////////////

inline Viewport::Viewport()
{
  frame = ospcommon::affine3f::translate(from) *
          ospcommon::affine3f(ospcommon::one);
}

inline void Viewport::setUp(const ospcommon::vec3f &vec)
{
  up = vec;
  snapUp();
  modified = true;
}

inline void Viewport::snapUp()
{
  if(fabsf(dot(up,frame.l.vz)) < 1e-3f)
    return;

  frame.l.vx = normalize(cross(frame.l.vy,up));
  frame.l.vz = normalize(cross(frame.l.vx,frame.l.vy));
  frame.l.vy = normalize(cross(frame.l.vz,frame.l.vx));
}

inline std::ostream &operator<<(std::ostream &o, const Viewport &viewport)
{
  o <<  "-vp " << viewport.from.x << " "
               << viewport.from.y << " "
               << viewport.from.z
    << " -vi " << viewport.at.x   << " "
               << viewport.at.y   << " "
               << viewport.at.z
    << " -vu " << viewport.up.x   << " "
               << viewport.up.y   << " "
               << viewport.up.z
    << std::endl;

  return o;
}
