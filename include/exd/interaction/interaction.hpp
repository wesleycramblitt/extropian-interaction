#pragma once

// ── Events ──
#include <exd/interaction/event.hpp>

// ── Hit resolution ──
#include <exd/interaction/hit.hpp>

// ── Gestures ──
#include <exd/interaction/gestures.hpp>

// ── Interaction state machines ──
#include <exd/interaction/interaction_state.hpp>

// ── Commands & undo/redo ──
#include <exd/interaction/command.hpp>

// ── Edit operations (chart, table, text, node) ──
#include <exd/interaction/edit_ops.hpp>

// ── Data graph (sources, paths, notifications) ──
#include <exd/interaction/data_graph.hpp>

// ── Data binding (field ← source path + transform) ──
#include <exd/interaction/binding.hpp>

// ── Binding resolver (propagation → dirty) ──
#include <exd/interaction/binding_resolver.hpp>

// ── Context focus (breadcrumb, drill in/out) ──
#include <exd/interaction/focus.hpp>

// ── Camera framing (bounds → camera pose) ──
#include <exd/interaction/camera_framing.hpp>

// ── View synchronization (brushing/linking) ──
#include <exd/interaction/view_sync.hpp>

namespace exd::interaction
{
    // Version info
    constexpr int VERSION_MAJOR = 0;
    constexpr int VERSION_MINOR = 1;
    constexpr int VERSION_PATCH = 0;
}
