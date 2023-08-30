#pragma once
#include "gl_driver.h"

#include <fstream>
#include <iostream>
#include <thread>

#define MAXFRAMES 1048576

class GLDump
{
  typedef struct 
  {
    int drawcall_count;
  } FrameData;

public:
  static GLDump * Ints();
  
  void Dumper();

  void StartDumper();

  /* 累加 drawcall */
  void AccDrawcall();

  void ResetFrameData();
  
private:
  GLDump();
  virtual ~GLDump() {}
  
  std::thread writerThread;

public:
  int m_current_frame = -1;
  int data2serialize = -1;
  FrameData *m_current_framedata;
  FrameData m_framedatas[MAXFRAMES];
};
