#include "precompiled.h"
#include "gl_dump.h"

GLDump::GLDump()
{
  RDCLOG("new GLDump");
}

GLDump *GLDump::Ints()
{
  static GLDump instance;
  return &instance;
}


void GLDump::Dumper()
{
  rdcstr dumpFile = FileIO::GetAppFolderFilename("renderdoc.txt");
  std::ofstream file(dumpFile.c_str(), std::ios::app);
  int serialized = 0;
  if (!file.is_open())
  {
    RDCLOG("%s is not open.", dumpFile.c_str());
    return;
  }

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
      
    int d2s = data2serialize;
    RDCLOG("data2serialize %d", d2s);
    RDCLOG("GLDump::Inst().data2serialize %d", data2serialize);
    RDCLOG("dump = %p", this);
    if (d2s > serialized)
    {
      for (; serialized < d2s; serialized++)
      {
        file << m_framedatas[serialized].drawcall_count << " ";
        //std::cout << GLDump::Inst().m_framedatas[serialized].drawcall_count << " ";
      }
      file << std::endl;
      //std::cout << std::endl;
      file.flush();
      RDCLOG("flush %d", serialized);
    }
  }
}

void GLDump::StartDumper()
{
  writerThread = std::thread(&GLDump::Dumper, this);
  RDCLOG("StartDumper dump = %p", this);
}

void GLDump::AccDrawcall()
{
  if (m_current_framedata != NULL)
    m_current_framedata->drawcall_count++;
}

void GLDump::ResetFrameData()
{
  if (m_current_frame == 0)
    StartDumper();
  if (m_current_frame >= 0)
  {
    //RDCLOG("frame %d, drawcall %d", m_current_frame, m_current_framedata->drawcall_count);
    if ((m_current_frame % 500) == 0)
    {
      RDCLOG("Writing ...");
      data2serialize = m_current_frame;
      RDCLOG("data2serialize %d, m_current_frame %d dump = %p", data2serialize, m_current_frame, this);
    }
  }
  m_current_frame++;
  m_current_framedata = &m_framedatas[m_current_frame];
}
