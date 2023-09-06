#pragma once
#include "gl_driver.h"

#include <fstream>
#include <iostream>
#include <thread>
#include <set>
#include <map>


#define MAXFRAMES 1048576

class GLDump
{
  typedef struct 
  {
    int drawcall_count;
    int texture_count;
    size_t texture_size;
  } FrameData;

  typedef struct
  {
    GLuint textureId;
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLenum type;
  } TextureInfo;

public:
  static GLDump * Ints();
  
  void Dumper();

  void StartDumper();

  void RecordDrawcall();

  // =========================================================
  // Texture
  // =========================================================
  void RecordTexture(WrappedOpenGL *m_pDriver, ResourceId id);
  
  void CacheTextureMemory(WrappedOpenGL *m_pDriver, ResourceId id, GLenum target, GLsizei levels,
                                        GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

  void CacheBufferTextureMemory(WrappedOpenGL *m_pDriver, ResourceId id, ResourceId bufid);

  static size_t CalcTextureMemory(GLsizei w, GLsizei h, GLsizei d, GLenum internalformat, uint32_t levels);

  size_t CalcTextureTotalMemory();

  // =========================================================
  // Frame Buffer
  // =========================================================
  void RecordFrameBufferTexture(WrappedOpenGL *m_pDriver, ResourceId id, ResourceId texid);
  void RecordFrameBuffer(WrappedOpenGL *m_pDriver, ResourceId id);

  // =========================================================
  // Buffer
  // =========================================================
  void RecordBuffer(WrappedOpenGL *m_pDriver, ResourceId id);

  void CacheBufferMemory(WrappedOpenGL *m_pDriver, ResourceId record, GLsizeiptr size);

  size_t CalcBufferTotalMemory();

  
  void ResetFrameData(WrappedOpenGL *m_pDriver, size_t backbufferColorSize);
  
private:
  GLDump();
  virtual ~GLDump() {}
  std::thread writerThread;

public:
  int m_current_frame = -1;
  int data2serialize = -1;
  FrameData m_framedatas[MAXFRAMES];
  FrameData *m_current_framedata;
  std::set<ResourceId> textures;
  std::map<ResourceId, std::set<ResourceId>> framebufferTextures;
  std::map<ResourceId, size_t> textureUsage;
  std::set<ResourceId> buffers;
  std::map<ResourceId, size_t> bufferUsage;
};
