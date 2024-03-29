#pragma once

#include "gfx/quad.hpp"
#include "gfx/render_texture.hpp"
#include "glm/ext/vector_float2.hpp"
#include "nvim/parse.hpp"
#include "editor/grid.hpp"
#include "webgpu/webgpu_cpp.h"
#include <map>

enum class Anchor { NW, NE, SW, SE };

struct FloatData {
  // Win* anchorWin;

  // Anchor anchor;
  // int anchorGrid;
  // float anchorRow;
  // float anchorCol;

  bool focusable;
  int zindex;
};

struct Win {
  Grid& grid;

  int startRow;
  int startCol;

  int width;
  int height;

  int hidden;

  // exists only if window is floating
  std::optional<FloatData> floatData;

  // rendering data
  glm::vec2 pos;
  glm::vec2 size;

  RenderTexture renderTexture;

  wgpu::TextureView maskTextureView;
  wgpu::Buffer maskPosBuffer;
  wgpu::BindGroup cursorBG;

  QuadRenderData<RectQuadVertex> rectData;
  QuadRenderData<TextQuadVertex> textData;
};

struct WinManager {
  GridManager* gridManager;
  glm::vec2 gridSize;
  float dpiScale;

  // not unordered because telescope float background overlaps text
  // so have to render floats in reverse order else text will be covered
  std::map<int, Win> windows;

  void InitRenderData(Win& win);
  void UpdateRenderData(Win& win);

  void Pos(const WinPos& e);
  void FloatPos(const WinFloatPos& e);
  void ExternalPos(const WinExternalPos& e);
  void Hide(const WinHide& e);
  void Close(const WinClose& e);
  void MsgSet(const MsgSetPos& e);
  void Viewport(const WinViewport& e);
  void Extmark(const WinExtmark& e);

  int activeWinId = 0;
  Win* GetActiveWin();
};
