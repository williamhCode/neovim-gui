#pragma once

#include "GLFW/glfw3.h"
#include "glm/ext/vector_uint2.hpp"

#include "gfx/context.hpp"

#include <atomic>
#include <functional>
#include <string>

struct Window {
  static inline WGPUContext _ctx;
  GLFWwindow* window;
  glm::uvec2 size;
  glm::uvec2 fbSize;

  std::function<void(int, int, int, int)> keyCallback = nullptr;
  std::function<void(unsigned int)> charCallback = nullptr;
  std::function<void(int, int, int)> mouseButtonCallback = nullptr;
  std::function<void(double, double)> cursorPosCallback = nullptr;
  std::function<void(int, int)> windowSizeCallback = nullptr;
  std::function<void(int, int)> framebufferSizeCallback = nullptr;
  std::function<void()> windowRefreshCallback = nullptr;

  Window() = default;
  Window(glm::uvec2 size, const std::string& title, wgpu::PresentMode presentMode);
  ~Window();

  void SetShouldClose(bool value);
  bool ShouldClose();

  void PollEvents();
  void WaitEvents();
  bool KeyPressed(int key);
  bool KeyReleased(int key);
};
