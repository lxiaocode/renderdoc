#include "precompiled.h"
#include "gl_dump.h"
#include <vector>

#include "gl_replay.h"

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
}

/* 计算每帧 DrawCall 数量 */
void GLDump::RecordDrawcall()
{
  if (m_current_framedata != NULL)
    m_current_framedata->drawcall_count++;
}

/* 计算每帧 BindTexture 数量 */
void GLDump::RecordTexture(WrappedOpenGL *m_pDriver, ResourceId id)
{
  if (m_current_framedata != NULL)
  {
    textures.insert(id);
  }
}

void GLDump::RecordFrameBufferTexture(WrappedOpenGL *m_pDriver, ResourceId id, ResourceId texid)
{
  if (framebufferTextures.find(id) == framebufferTextures.end())
  {
    framebufferTextures.insert(std::pair<ResourceId, std::set<ResourceId>>(id, std::set<ResourceId>{texid}));
  }
  else
  {
    framebufferTextures[id].insert(texid);
  }
}

void GLDump::RecordFrameBuffer(WrappedOpenGL *m_pDriver, ResourceId id)
{
  for (ResourceId texid : framebufferTextures[id])
  {
    RecordTexture(m_pDriver, texid);
  }
}

void GLDump::RecordBuffer(WrappedOpenGL *m_pDriver, ResourceId id)
{
  if (m_current_framedata != NULL)
  {
    buffers.insert(id);
  }
}

void GLDump::CacheTextureMemory(WrappedOpenGL *m_pDriver, ResourceId id, GLenum target, GLsizei levels,
                                        GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
  if (textureUsage.find(id) == textureUsage.end())
  {
    size_t texSize = CalcTextureMemory(width, height, depth, internalformat, levels);
    // RDCLOG("resourceId %d, levels %d, width %d, height %d, depth %d, texSize %d", id.id, levels, width, height, depth, texSize);
    textureUsage.insert(std::pair<ResourceId, size_t>(id, texSize));
  }
}

void GLDump::CacheBufferTextureMemory(WrappedOpenGL *m_pDriver, ResourceId id, ResourceId bufid)
{
  if (textureUsage.find(id) == textureUsage.end())
  {
    size_t bufsize = 0;
    if (bufferUsage.find(bufid) != bufferUsage.end())
    {
      bufsize = bufferUsage[bufid];
    }
    textureUsage.insert(std::pair<ResourceId, size_t>(id, bufsize));
  }
}

void GLDump::CacheBufferMemory(WrappedOpenGL *m_pDriver, ResourceId id, GLsizeiptr size)
{
  if (bufferUsage.find(id) == bufferUsage.end())
  {
    bufferUsage.insert(std::pair<ResourceId, size_t>(id, size));
  }
}

size_t GLDump::CalcTextureTotalMemory()
{
  size_t ret = 0;
  RDCLOG("CalcTextureTotalMemory Start");
  for (ResourceId id : textures)
  {
    size_t m = textureUsage[id];
    ret += m;
    RDCLOG("%s, size %d", ToStr(id).c_str(), m);
  }
  RDCLOG("CalcTextureTotalMemory End");
  return ret;
}

size_t GLDump::CalcBufferTotalMemory()
{
  size_t ret = 0;
  RDCLOG("CalcBufferTotalMemory Start");
  for (ResourceId id : buffers)
  {
    size_t m = bufferUsage[id];
    ret += m;
    RDCLOG("%s, size %d", ToStr(id).c_str(), m);
  }
  RDCLOG("CalcBufferTotalMemory End");
  return ret;
}

void GLDump::ResetFrameData(WrappedOpenGL *m_pDriver, size_t backbufferColorSize)
{
//  if (m_current_frame == 0)
//    StartDumper();
  if (m_current_frame >= MAXFRAMES - 1)
    return;
  if (m_current_frame >= 0)
  {
    m_current_framedata->texture_count = textures.size();
    m_current_framedata->texture_size = CalcTextureTotalMemory() + (backbufferColorSize * 4);
    RDCLOG("frame %d, drawcall %d, textures %d, CountTextureTotalMemory %.2f MB, CalcBufferTotalMemory %.2f MB", m_current_frame, m_current_framedata->drawcall_count, m_current_framedata->texture_count, (float)m_current_framedata->texture_size / (1024.0f * 1024.0f), (float)CalcBufferTotalMemory() / (1024.0f * 1024.0f));
    if ((m_current_frame % 500) == 0)
    {
      data2serialize = m_current_frame;
    }
  }
  m_current_frame++;
  m_current_framedata = &m_framedatas[m_current_frame];
  textures.clear();
  buffers.clear();
}

size_t GLDump::CalcTextureMemory(GLsizei w, GLsizei h, GLsizei d, GLenum internalformat, uint32_t levels)
{
  size_t byteSize = 0;
  for(uint32_t m = 0; m < levels; m++)
  {
    byteSize += (uint64_t)GetCompressedByteSize(RDCMAX(1, w >> m), RDCMAX(1, h >> m), d, internalformat);
  }

  return byteSize;
}
