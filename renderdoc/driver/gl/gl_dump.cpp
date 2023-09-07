#include "precompiled.h"
#include "gl_dump.h"
#include <chrono>
#include <sstream>

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
  auto now = std::chrono::system_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
  std::stringstream filename;
  filename << "RenderdocDumper-" << ms << ".txt";
  
  rdcstr dumpFile = FileIO::GetAppFolderFilename(filename.str().c_str());
  std::ofstream file(dumpFile.c_str(), std::ios::app);
  if (!file.is_open())
  {
    RDCLOG("%s is not open.", dumpFile.c_str());
    return;
  }

  while (true)
  {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    RDCLOG("Waiting for serizlization %d", m_FrameDatas->size());
    if (m_FrameDatas->size() > 300)
    {
      for (int i = 0; i < 300; i++)
      {
        FrameData data = m_FrameDatas->dequeue();
        file << data.frame << ",";
        file << data.drawcall_count << ",";
        file << data.texture_count << ",";
        file << data.texture_size << ",";
        file << data.buffer_count << ",";
        file << data.buffer_size << "\n";
      }
      //std::cout << std::endl;
      file.flush();
      RDCLOG("flush %d", m_FrameDatas->getFront().frame);
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
  if (m_CurrentFrameData != NULL)
    m_CurrentFrameData->drawcall_count++;
}

/* 计算每帧 BindTexture 数量 */
void GLDump::RecordTexture(WrappedOpenGL *m_pDriver, ResourceId id)
{
  if (m_CurrentFrameData != NULL)
  {
    m_TmpTextures.insert(id);
  }
}

void GLDump::RecordFrameBufferTexture(WrappedOpenGL *m_pDriver, ResourceId id, ResourceId texid)
{
  if (m_CacheFrameBuffers.find(id) == m_CacheFrameBuffers.end())
  {
    m_CacheFrameBuffers.insert(std::pair<ResourceId, std::set<ResourceId>>(id, std::set<ResourceId>{texid}));
  }
  else
  {
    m_CacheFrameBuffers[id].insert(texid);
  }
}

void GLDump::RecordFrameBuffer(WrappedOpenGL *m_pDriver, ResourceId id)
{
  for (ResourceId texid : m_CacheFrameBuffers[id])
  {
    RecordTexture(m_pDriver, texid);
  }
}

void GLDump::RecordBuffer(WrappedOpenGL *m_pDriver, ResourceId id)
{
  if (m_CurrentFrameData != NULL)
  {
    m_TmpBuffers.insert(id);
  }
}

void GLDump::CacheTextureMemory(WrappedOpenGL *m_pDriver, ResourceId id, GLenum target, GLsizei levels,
                                        GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
  if (m_CacheTextures.find(id) == m_CacheTextures.end())
  {
    size_t texSize = CalcTextureMemory(width, height, depth, internalformat, levels);
    // RDCLOG("%s, levels %d, width %d, height %d, depth %d, texSize %d", ToStr(id).c_str(), levels, width, height, depth, texSize);

    if (target == eGL_TEXTURE_CUBE_MAP)
    {
      texSize = texSize * 6;
    }
    m_CacheTextures.insert(std::pair<ResourceId, size_t>(id, texSize));
  }
}

void GLDump::CacheBufferTextureMemory(WrappedOpenGL *m_pDriver, ResourceId id, ResourceId bufid)
{
  if (m_CacheTextures.find(id) == m_CacheTextures.end())
  {
    size_t bufsize = 0;
    if (m_CacheBuffers.find(bufid) != m_CacheBuffers.end())
    {
      bufsize = m_CacheBuffers[bufid];
    }
    m_CacheTextures.insert(std::pair<ResourceId, size_t>(id, bufsize));
  }
}

void GLDump::CacheBufferMemory(WrappedOpenGL *m_pDriver, ResourceId id, GLsizeiptr size)
{
  if (m_CacheBuffers.find(id) == m_CacheBuffers.end())
  {
    m_CacheBuffers.insert(std::pair<ResourceId, size_t>(id, size));
  }
}

size_t GLDump::CalcTextureTotalMemory()
{
  size_t ret = 0;
  //RDCLOG("CalcTextureTotalMemory Start");
  for (ResourceId id : m_TmpTextures)
  {
    ret += m_CacheTextures[id];
    //RDCLOG("%s : %d", ToStr(id).c_str(), m_CacheTextures[id]);
  }
  //RDCLOG("CalcTextureTotalMemory End");
  return ret;
}

size_t GLDump::CalcBufferTotalMemory()
{
  size_t ret = 0;
  for (ResourceId id : m_TmpBuffers)
  {
    ret += m_CacheBuffers[id];
  }
  return ret;
}

void GLDump::ResetFrameData(WrappedOpenGL *m_pDriver, size_t backbufferColorSize)
{
  if (m_CurrentFrame == 0)
    StartDumper();
  if (m_FrameDatas->isFull())
    return;
  if (m_CurrentFrame > 0)
  {
    m_CurrentFrameData->frame = m_CurrentFrame;
    m_CurrentFrameData->texture_count = m_TmpTextures.size() + 1;
    m_CurrentFrameData->texture_size = CalcTextureTotalMemory() + (backbufferColorSize * 4);
    m_CurrentFrameData->buffer_count = m_TmpBuffers.size();
    m_CurrentFrameData->buffer_size = CalcBufferTotalMemory();
    RDCLOG("frame %d, drawcall %d, textures %d, CountTextureTotalMemory %.2f MB, buffers %d, CalcBufferTotalMemory %.2f MB", m_CurrentFrame, m_CurrentFrameData->drawcall_count, m_CurrentFrameData->texture_count, (float)m_CurrentFrameData->texture_size / (1024.0f * 1024.0f), m_CurrentFrameData->buffer_count, (float)m_CurrentFrameData->buffer_size / (1024.0f * 1024.0f));
  }
  m_CurrentFrame++;
  //m_CurrentFrameData = &m_framedatas[m_CurrentFrame];
  m_CurrentFrameData = m_FrameDatas->enqueueRef();
  m_TmpTextures.clear();
  m_TmpBuffers.clear();
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
