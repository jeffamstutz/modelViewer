
#pragma once

#include <cstdlib>

struct ViewerConfig
{
  bool alwaysRedraw;
  bool doShadows;
  bool showDepthBuffer;
  int g_benchWarmup;
  int g_benchFrames;

  const char *outFileName;
  size_t numAccumsFrameInFileOutput;
  size_t numSPPinFileOutput;

  int maxAccum;

  ViewerConfig() :
    alwaysRedraw(false),
    doShadows(true),
    showDepthBuffer(false),
    g_benchWarmup(0),
    g_benchFrames(0),
    outFileName(NULL),
    numAccumsFrameInFileOutput(1),
    numSPPinFileOutput(1),
    maxAccum(64)
  {
  }
};
