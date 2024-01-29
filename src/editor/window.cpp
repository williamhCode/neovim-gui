#include "window.hpp"

#include <iostream>
#include <ostream>

const WGPUContext& ctx = Window::ctx;

static void
KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  Window& win = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  win.events.push_back(
    {Window::EventType::Key, Window::KeyData{key, scancode, action, mods}}
  );
}

static void CharCallback(GLFWwindow* window, unsigned int codepoint) {
  Window& win = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  win.events.push_back(
    {Window::EventType::Char, Window::CharData{codepoint}}
  );
}

static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
  Window& win = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  win.events.push_back(
    {Window::EventType::MouseButton, Window::MouseButtonData{button, action, mods}}
  );
}

static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
  Window& win = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  win.events.push_back(
    {Window::EventType::CursorPos, Window::CursorPosData{xpos, ypos}}
  );
}

static void WindowSizeCallback(GLFWwindow* window, int width, int height) {
  Window& win = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  win.size = {width, height};
}

static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
  Window& win = *reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
  // win.ctx.Resize({width, height});
}

Window::Window(glm::uvec2 size, const std::string& title, wgpu::PresentMode presentMode)
    : size(size) {
  if (!glfwInit()) {
    std::cerr << "Could not initialize GLFW!" << std::endl;
    std::exit(1);
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow(size.x, size.y, title.c_str(), NULL, NULL);
  if (!window) {
    std::cerr << "Could not open window!" << std::endl;
    std::exit(1);
  }

  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, KeyCallback);
  glfwSetCharCallback(window, CharCallback);
  glfwSetMouseButtonCallback(window, MouseButtonCallback);
  glfwSetCursorPosCallback(window, CursorPosCallback);
  glfwSetWindowSizeCallback(window, WindowSizeCallback);
  glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

  // webgpu ------------------------------------
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  ctx = WGPUContext(window, {width, height}, presentMode);
}

Window::~Window() {
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Window::SetShouldClose(bool value) {
  glfwSetWindowShouldClose(window, value);
}

bool Window::ShouldClose() {
  return glfwWindowShouldClose(window);
}

void Window::PollEvents() {
  events.clear();
  glfwPollEvents();
}

void Window::WaitEvents() {
  events.clear();
  glfwWaitEvents();
}

bool Window::KeyPressed(int key) {
  return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Window::KeyReleased(int key) {
  return glfwGetKey(window, key) == GLFW_RELEASE;
}