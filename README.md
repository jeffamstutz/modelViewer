ospDebugViewer
==============

External version of the ospModelViewer from the ospray source tree with added scripting and benchmark capabilities.

Getting Started
--------------------------

This project requires a valid install of OSPRay, which can be found at
https://github.com/ospray/OSPRay. There are instructions there for how to build
OSPRay from source. Alternatively, binary releases posted with each OSPRay release
will also work.

Currently ospDebugViewer is only supported on Linux and OS X using either gcc or clang.
In principle, ospDebugViewer should work on Windows, but it is not officially tested or
supported at this time. Additionally, the scripting library used does not
support compiling with icc at the moment. However, OSPRay itself can be compiled with
icc and used in ospDebugViewer.

One you have a valid OSPRay installed, building ospDebugViewer is fairly
straightforward. First, you will have to have GLUT installed, which can be
installed on Debian based Linux distros by installing the 'freeglut3-dev'
package, or yum based RHEL flavors with the 'freeglut-devel' package. On OS X,
GLUT can be found via Macports or Homebrew.

Once OSPRay and GLUT are installed, you're ready to compile ospDebugViewer. CMake
looks for your OSPRay install via the 'ospray_DIR' environment variable, which
should point to the root of your OSPRay install. Thus on a machine where OSPRay
is installed into '/opt/ospray', compiling ospDebugViewer is a simple as:

    user@mymachine[~/Projects/ospDebugViewer]: mkdir build
    user@mymachine[~/Projects/ospDebugViewer/build]: cd build
    user@mymachine[~/Projects/ospDebugViewer/build]: export ospray_DIR=/opt/ospray
    user@mymachine[~/Projects/ospDebugViewer/build]: cmake ..
    user@mymachine[~/Projects/ospDebugViewer/build]: make

The 'scripts/' directory in the ospDebugViewer source tree has some scripts
which know how to go fetch OSPRay, compile it with gcc, and build
ospDebugViewer with it. You can run the 'scripts/build_linux.sh' script to do
this, where it will create an 'ospray' directory containing ospray source,
build, and install, and a 'build' directory where ospDebugViewer is compiled.

