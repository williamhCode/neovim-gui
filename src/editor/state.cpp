#include "state.hpp"
#include "glm/ext/vector_float4.hpp"
#include "utils/variant.hpp"
#include <vector>

static auto IntToColor(uint32_t color) {
  return glm::vec4(
    ((color >> 16) & 0xff) / 255.0f,
    ((color >> 8) & 0xff) / 255.0f,
    (color & 0xff) / 255.0f,
    1.0f
  );
};

void ProcessRedrawEvents(RedrawState& redrawState, EditorState& editorState) {
  for (int i = 0; i < redrawState.numFlushes; i++) {
    auto& redrawEvents = redrawState.eventsQueue.front();
    // to process grid events first then window
    std::vector<RedrawEvent*> gridEvents;
    std::vector<RedrawEvent*> winEvents;
    for (auto& event : redrawEvents) {
      std::visit(overloaded{
        [&](SetTitle& e) {
          // LOG("set_title");
        },
        [&](SetIcon& e) {
          // LOG("set_icon");
        },
        [&](ModeInfoSet& e) {
          for (auto& elem : e.modeInfo) {
            auto& modeInfo = editorState.modeInfoList.emplace_back();
            for (auto& [key, value] : elem) {
              if (key == "cursor_shape") {
                auto shape = value.as_string();
                if (shape == "block") {
                  modeInfo.cursorShape = CursorShape::Block;
                } else if (shape == "horizontal") {
                  modeInfo.cursorShape = CursorShape::Horizontal;
                } else if (shape == "vertical") {
                  modeInfo.cursorShape = CursorShape::Vertical;
                } else {
                  LOG_WARN("unknown cursor shape: {}", shape);
                }
              } else if (key == "cell_percentage") {
                modeInfo.cellPercentage = value.as_uint64_t();
              } else if (key == "blinkwait") {
                modeInfo.blinkwait = value.as_uint64_t();
              } else if (key == "blinkon") {
                modeInfo.blinkon = value.as_uint64_t();
              } else if (key == "blinkoff") {
                modeInfo.blinkoff = value.as_uint64_t();
              } else if (key == "attr_id") {
                modeInfo.attrId = value.as_uint64_t();
              }
            }
          }
        },
        [&](OptionSet& e) {
          // LOG("option_set");
        },
        [&](Chdir& e) {
          // LOG("chdir");
        },
        [&](ModeChange& e) {
          editorState.modeIdx = e.modeIdx;
          editorState.cursor.SetMode(&editorState.modeInfoList[e.modeIdx]);
        },
        [&](MouseOn&) {
          // LOG("mouse_on");
        },
        [&](MouseOff&) {
          // LOG("mouse_off");
        },
        [&](BusyStart&) {
          // LOG("busy_start");
        },
        [&](BusyStop&) {
          // LOG("busy_stop");
        },
        [&](UpdateMenu&) {
          // LOG("update_menu");
        },
        [&](DefaultColorsSet& e) {
          // LOG("default_colors_set");
          auto& hl = editorState.hlTable[0];
          hl.foreground = IntToColor(e.rgbFg);
          hl.background = IntToColor(e.rgbBg);
          hl.special = IntToColor(e.rgbSp);
        },
        [&](HlAttrDefine& e) {
          // LOG("hl_attr_define");
          auto& hl = editorState.hlTable[e.id];
          for (auto& [key, value] : e.rgbAttrs) {
            if (key == "foreground") {
              hl.foreground = IntToColor(value.as_uint64_t());
            } else if (key == "background") {
              hl.background = IntToColor(value.as_uint64_t());
            } else if (key == "special") {
              hl.special = IntToColor(value.as_uint64_t());
            } else if (key == "reverse") {
              hl.reverse = value.as_bool();
            } else if (key == "italic") {
              hl.italic = value.as_bool();
            } else if (key == "bold") {
              hl.bold = value.as_bool();
            } else if (key == "strikethrough") {
              hl.strikethrough = value.as_bool();
            } else if (key == "underline") {
              hl.underline = UnderlineType::Underline;
            } else if (key == "undercurl") {
              hl.underline = UnderlineType::Undercurl;
            } else if (key == "underdouble") {
              hl.underline = UnderlineType::Underdouble;
            } else if (key == "underdotted") {
              hl.underline = UnderlineType::Underdotted;
            } else if (key == "underdashed") {
              hl.underline = UnderlineType::Underdashed;
            } else if (key == "blend") {
              hl.blend = value.as_uint64_t();
            } else {
              LOG_WARN("unknown hl attr key: {}", key);
            }
          }
        },
        [&](HlGroupSet&) {
          // not needed to render grids, but used for rendering
          // own elements with consistent highlighting
        },
        [&](Flush&) {
          // process grid and window events
          for (auto event : gridEvents) {
            std::visit(overloaded{
              [&](GridResize& e) {
                editorState.gridManager.Resize(e);
                // default window events not send by nvim
                if (e.grid == 1) {
                  editorState.winManager.Pos({1, {}, 0, 0, e.width, e.height});
                }
              },
              [&](GridClear& e) {
                editorState.gridManager.Clear(e);
              },
              [&](GridCursorGoto& e) {
                editorState.gridManager.CursorGoto(e);
                editorState.winManager.activeWinId = e.grid;
              },
              [&](GridLine& e) {
                editorState.gridManager.Line(e);
              },
              [&](GridScroll& e) {
                editorState.gridManager.Scroll(e);
              },
              [&](GridDestroy& e) {
                editorState.gridManager.Destroy(e);
                // TODO: file bug report, win_close not called after tabclose
                // temp fix for bug
                editorState.winManager.Close({e.grid});
              },
              [&](auto&) {
                LOG_WARN("unknown event");
              }
            }, *event);
          }
          for (auto& event : winEvents) {
            std::visit(overloaded{
              [&](WinPos& e) {
                editorState.winManager.Pos(e);
              },
              [&](WinFloatPos& e) {
                editorState.winManager.FloatPos(e);
              },
              [&](WinExternalPos& e) {
              },
              [&](WinHide& e) {
                editorState.winManager.Hide(e);
              },
              [&](WinClose& e) {
                editorState.winManager.Close(e);
              },
              [&](MsgSetPos& e) {
                editorState.winManager.MsgSet(e);
              },
              [&](WinViewport& e) {
                editorState.winManager.Viewport(e);
              },
              [&](WinExtmark& e) {
              },
              [&](auto&) {
                LOG_WARN("unknown event");
              }
            }, *event);
          }
          gridEvents.clear();
          winEvents.clear();
        },
        [&](GridResize& e) {
          gridEvents.push_back((RedrawEvent*)&e);
        },
        [&](GridClear& e) {
          gridEvents.push_back((RedrawEvent*)&e);
        },
        [&](GridCursorGoto& e) {
          gridEvents.push_back((RedrawEvent*)&e);
        },
        [&](GridLine& e) {
          gridEvents.push_back((RedrawEvent*)&e);
        },
        [&](GridScroll& e) {
          gridEvents.push_back((RedrawEvent*)&e);
          // editorState.gridManager.Scroll(e);
        },
        [&](GridDestroy& e) {
          gridEvents.push_back((RedrawEvent*)&e);
        },
        [&](WinPos& e) {
          winEvents.push_back((RedrawEvent*)&e);
        },
        [&](WinFloatPos& e) {
          winEvents.push_back((RedrawEvent*)&e);
        },
        [&](WinExternalPos& e) {
          winEvents.push_back((RedrawEvent*)&e);
        },
        [&](WinHide& e) {
          winEvents.push_back((RedrawEvent*)&e);
        },
        [&](WinClose& e) {
          winEvents.push_back((RedrawEvent*)&e);
        },
        [&](MsgSetPos& e) {
          winEvents.push_back((RedrawEvent*)&e);
        },
        [&](WinViewport& e) {
          winEvents.push_back((RedrawEvent*)&e);
        },
        [&](WinExtmark& e) {
          winEvents.push_back((RedrawEvent*)&e);
        },
        [&](auto&) {
          LOG_WARN("unknown event");
        },
      }, event);
    }
    redrawState.eventsQueue.pop_front();
  }
}
