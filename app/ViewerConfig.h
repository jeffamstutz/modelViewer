
#pragma once

#include <cstdlib>

struct ViewerConfig
{
  bool alwaysRedraw;
  bool doShadows;
  bool showDepthBuffer;
  int benchWarmup;
  int benchFrames;

  const char *outFileName;
  size_t numAccumsFrameInFileOutput;
  size_t numSPPinFileOutput;

  bool verboseOutput;

  ViewerConfig() :
    alwaysRedraw(true),
    doShadows(true),
    showDepthBuffer(false),
    benchWarmup(0),
    benchFrames(0),
    outFileName(NULL),
    numAccumsFrameInFileOutput(1),
    numSPPinFileOutput(1),
    verboseOutput(false)
  {
  }
};
