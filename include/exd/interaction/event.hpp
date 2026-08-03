#pragma once

#include <exd/math/vec3.hpp>

#include <string>
#include <variant>
#include <cstdint>

namespace exd::interaction
{

// ── Entity identifiers ──
using VisualId = uint64_t;
constexpr VisualId kInvalidVisualId = 0;

// ── Pointer event ──

struct PointerEvent
{
    enum class Type { Move, Down, Up, Scroll, Enter, Leave };
    Type type = Type::Move;
    math::Vec3f position = {0.0f, 0.0f, 0.0f};
    math::Vec3f delta = {0.0f, 0.0f, 0.0f};
    int button = 0;
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
};

// ── Keyboard event ──

struct KeyboardEvent
{
    enum class Type { Down, Up, Repeat };
    Type type = Type::Down;
    int key = 0;
    int scancode = 0;
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
};

// ── High‑level visual events ──

struct VisualHovered      { VisualId visualId = kInvalidVisualId; math::Vec3f position = {}; };
struct VisualUnhovered    { VisualId visualId = kInvalidVisualId; };
struct VisualFocused      { VisualId visualId = kInvalidVisualId; };
struct VisualUnfocused    { VisualId visualId = kInvalidVisualId; };
struct VisualSelected     { VisualId visualId = kInvalidVisualId; };
struct VisualDeselected   { VisualId visualId = kInvalidVisualId; };
struct VisualActivated    { VisualId visualId = kInvalidVisualId; };

struct VisualDragged
{
    VisualId visualId = kInvalidVisualId;
    math::Vec3f start = {};
    math::Vec3f current = {};
    math::Vec3f delta = {};
};

struct VisualResized
{
    VisualId visualId = kInvalidVisualId;
    math::Vec3f newSize = {};
};

struct VisualTransformChanged { VisualId visualId = kInvalidVisualId; };

struct ScrollChanged
{
    VisualId visualId = kInvalidVisualId;
    math::Vec3f position = {};
};

struct TextEdited
{
    VisualId visualId = kInvalidVisualId;
    std::string text;
};

// ── Chart & data‑specific events ──

struct DataPointHovered
{
    VisualId visualId = kInvalidVisualId;
    uint32_t seriesIndex = 0;
    uint32_t pointIndex = 0;
    math::Vec3f value = {};
};

struct AxisBrushed
{
    VisualId visualId = kInvalidVisualId;
    uint32_t axisIndex = 0;
    double selectionMin = 0.0;
    double selectionMax = 0.0;
};

struct SeriesReordered
{
    VisualId visualId = kInvalidVisualId;
    uint32_t fromIndex = 0;
    uint32_t toIndex = 0;
};

struct ValueScrubbed
{
    VisualId visualId = kInvalidVisualId;
    uint32_t seriesIndex = 0;
    uint32_t pointIndex = 0;
    double newValue = 0.0;
};

struct CellEdited
{
    VisualId visualId = kInvalidVisualId;
    uint32_t row = 0;
    uint32_t column = 0;
    std::string value;
};

// ── Variant event ──

using VisualEvent = std::variant<
    VisualHovered,
    VisualUnhovered,
    VisualFocused,
    VisualUnfocused,
    VisualSelected,
    VisualDeselected,
    VisualActivated,
    VisualDragged,
    VisualResized,
    VisualTransformChanged,
    ScrollChanged,
    TextEdited,
    DataPointHovered,
    AxisBrushed,
    SeriesReordered,
    ValueScrubbed,
    CellEdited
>;

// ── Interaction config ──

struct InteractionConfig
{
    bool hoverable = false;
    bool focusable = false;
    bool selectable = false;
    bool draggable = false;
    bool resizable = false;
    bool scrollable = false;
    bool blocksEventPropagation = false;
};

}  // namespace exd::interaction
