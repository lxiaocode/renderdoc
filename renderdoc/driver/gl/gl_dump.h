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

  void AccDrawcall();

  // =========================================================
  // Texture
  // =========================================================
  void AccTexture(WrappedOpenGL *m_pDriver, ResourceId id);
  
  void CacheTextureMemory(WrappedOpenGL *m_pDriver, ResourceId record, GLenum target, GLsizei levels,
                                        GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);

  static size_t CalcTextureMemory(GLsizei w, GLsizei h, GLsizei d, GLenum internalformat, uint32_t levels);

  size_t CalcTextureTotalMemory();



  void ResetFrameData(WrappedOpenGL *m_pDriver, size_t backbufferColorSize);
  
private:
  GLDump();
  virtual ~GLDump() {}
  std::thread writerThread;
  std::map<ResourceId, TextureDescription> m_CachedTextures;

public:
  int m_current_frame = -1;
  int data2serialize = -1;
  FrameData m_framedatas[MAXFRAMES];
  FrameData *m_current_framedata;
  std::set<ResourceId> textures;
  std::map<ResourceId, size_t> textureUsage;
};
