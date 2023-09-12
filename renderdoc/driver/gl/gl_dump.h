﻿#pragma once
#include "gl_driver.h"

#include <fstream>
#include <iostream>
#include <thread>
#include <set>
#include <map>

template <typename T>
class CircularQueue
{
private:
  T* buffer;
  int capacity;
  int front;
  int rear;
  int count;

public:
  CircularQueue(int size)
  {
    capacity = size;
    buffer = new T[capacity];
    front = 0;
    rear = -1;
    count = 0;
  }

  ~CircularQueue()
  {
    delete[] buffer;
  }

  void enqueue(T item)
  {
    if (isFull())
    {
      return;
    }

    rear = (rear + 1) % capacity;
    buffer[rear] = item;
    count++;
  }

  T* enqueueRef()
  {
    if (isFull())
    {
      return NULL;
    }
    rear = (rear + 1) % capacity;
    count++;
    return &buffer[rear];
  }

  T dequeue()
  {
    if (isEmpty())
    {
      return T();
    }
    T ret = buffer[front];
    front = (front + 1) % capacity;
    count--;
    return ret;
  }

  T getFront()
  {
    if (isEmpty())
    {
      return T();
    }
    return buffer[front];
  }

  bool isEmpty()
  {
    return count == 0;
  }

  bool isFull()
  {
    return count == capacity;
  }

  int size()
  {
    return count;
  }
};

#define MAXFRAMES 3000

class GLDump
{
  struct FrameData
  {
    size_t frame;
    size_t drawcall_count;
    size_t texture_count;
    size_t texture_size;
    size_t buffer_count;
    size_t buffer_size;

    void Reset()
    {
      frame = 0;
      drawcall_count = 0;
      texture_count = 0;
      texture_size = 0;
      buffer_count = 0;
      buffer_size = 0;
    }
  };

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

  
  void ResetFrameData(WrappedOpenGL *m_pDriver, size_t backbufferColorSize, size_t backbufferDepthStencilSize);

  // =========================================================
  // Debug
  // =========================================================
  void SetCaptureState();
  void DebugLog();
  void DebugLogTextures();
  
private:
  GLDump();
  virtual ~GLDump() {}
  std::thread writerThread;
  std::thread debugThread;
  std::set<ResourceId> m_DebugTextures;

private:
  int m_IsCapture = 0;
  
  size_t m_CurrentFrame = 0;
  CircularQueue<FrameData> *m_FrameDatas = new CircularQueue<FrameData>(MAXFRAMES);

  std::map<ResourceId, std::set<ResourceId>> m_CacheFrameBuffers;
  std::map<ResourceId, size_t> m_CacheTextures;
  std::map<ResourceId, size_t> m_CacheBuffers;


  FrameData *m_CurrentFrameData;
  std::set<ResourceId> m_TmpTextures;
  std::set<ResourceId> m_TmpBuffers;


};
