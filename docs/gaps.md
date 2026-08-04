# Extropian Interaction — Gap Analysis

> Audit comparing the current codebase against `docs/plan.md`.

---

## A. Stub / TODO Implementations (Critical)

### 1. `hit.cpp` — `resolve_hit()` is a stub

```cpp
// TODO: walk render::Parent hierarchy chain to resolve VisualId
return HitResult{ hitEntity, position, depth };
```

Returns the hit entity as-is without walking the parent hierarchy chain. The module's core purpose — hit resolution — is non-functional.

### 2. `edit_ops.cpp` — Three commands are inert

| Command | execute() | undo() |
|---------|-----------|--------|
| `AppendDataPointCommand` | `// TODO: patch chart descriptor` | `// TODO: remove last point` |
| `RemoveDataPointCommand` | `// TODO: remove from chart series` | `// TODO: re-insert` |
| `ReorderSeriesCommand` | `// TODO: reorder chart series` | `// TODO: swap back` |

All three just call `doc_.markDirty()` with no actual mutation. They need access to a chart descriptor (possibly through the `IDocument` interface or `DataGraph`) to store old state and apply changes.

### 3. `camera_framing.cpp` — `orbitCamera()` is a no-op

Ignores both `deltaAzimuth` and `deltaElevation` parameters:
```cpp
CameraPose result = current;
(void)deltaAzimuth;
(void)deltaElevation;
return result;
```

Needs full spherical-to-Cartesian rotation math around the target point.

### 4. `view_sync.cpp` — Four methods are stubs

| Method | Status |
|--------|--------|
| `setBrushRange()` | No-op (`// TODO: set brush range, resolve visuals within range`) |
| `brushedVisuals()` | Returns static empty set |
| `setViewFilter()` | No-op (`// TODO: store and apply filter`) |
| `clearViewFilter()` | No-op (`// TODO: clear filter`) |

Needs per-view axis-range storage, spatial query against dataset bounds, and filter function storage.

---

## B. `EditTextCommand` — `oldText_` Never Captured

The constructor does not read the current text:
```cpp
EditTextCommand::EditTextCommand(IDocument& doc, VisualId visualId, std::string newText)
    : doc_(doc), visualId_(visualId), newText_(std::move(newText))
{}
```

`oldText_` remains empty. `undo()` sets text to `""` instead of restoring the original. Needs an `IDocument::getText(VisualId)` method or the constructor must read from the doc.

---

## C. Gestures: Recognizer Doesn't Emit Its Own Enum Members

`GestureKind` defines: `None, Tap, DoubleTap, LongPress, Drag, Brush, Scrub, Pinch, Pan`

The `feed()` method only produces: **Tap**, **DoubleTap**, **Drag**.

Missing:
- **LongPress** — no timeout-based detection
- **Brush** — no range-drag recognition
- **Scrub** — no value-drag recognition
- **Pinch** — no two-finger zoom (may be platform-dependent)
- **Pan** — no two-finger or middle-button pan

---

## D. InteractionState: Most Event Types Never Produced

The plan lists 17 `VisualEvent` variant members. Only 6 are emitted:

| Event | Status |
|-------|--------|
| `VisualHovered` | Emitted |
| `VisualUnhovered` | Emitted |
| `VisualFocused` | Emitted |
| `VisualSelected` | Emitted |
| `VisualDeselected` | Emitted |
| `VisualDragged` | Emitted |
| `VisualUnfocused` | **Not emitted** |
| `VisualActivated` | **Not emitted** |
| `VisualResized` | **Not emitted** |
| `VisualTransformChanged` | **Not emitted** |
| `ScrollChanged` | **Not emitted** |
| `TextEdited` | **Not emitted** |
| `DataPointHovered` | **Not emitted** |
| `AxisBrushed` | **Not emitted** |
| `SeriesReordered` | **Not emitted** |
| `ValueScrubbed` | **Not emitted** |
| `CellEdited` | **Not emitted** |

Additionally:
- `InteractionConfig::resizable`, `scrollable`, `blocksEventPropagation` are declared but never read.
- `resizing_` member exists but is never set.
- `currentActionId_` has a getter but no setter or logic to populate it.
- `PointerEvent::Type::Scroll` is defined in the enum but handled in neither `gestures.cpp` nor `interaction_state.cpp`.

---

## E. Transform System: Entirely Unimplemented

`TransformKind` enum and `TransformParams` struct are declared in `binding.hpp`:
- `Scale` — value × factor
- `Clamp` — min/max clamping
- `Normalize` — remap [inMin,inMax] → [outMin,outMax]
- `ElementWise` — per-element series transform
- `Aggregate` — sum/avg/min/max across series

**No implementation exists anywhere.** `BindingRegistry::resolveForSource()` passes raw values through without consulting `transform`. Every binding effectively uses `TransformKind::None`.

Need: transform functions (likely free functions or a `TransformApplier`), and `BindingRegistry` must invoke them during resolution.

---

## F. WASM Bridge: Not Built

Plan §6 documents a WASM bridge API:
```
interaction_feed_pointer(x, y, button, type)
interaction_get_event()
interaction_execute_command(cmd_json)
interaction_bind(visualId, fieldPath, sourcePath)
interaction_set_value(path, value)
```

Missing:
- No WASM bridge `.cpp` file with `EMSCRIPTEN_BINDINGS` or `extern "C"` exports
- No Emscripten CMake toolchain configuration
- No `BUILD_WASM` CMake option
- No serialization to/from JSON for events and commands

---

## G. `DataGraph::subscribe()` Ignores Path Prefix

```cpp
uint64_t DataGraph::subscribe(const std::string& /*path*/, DataChangeCallback callback)
```

The `path` parameter is ignored — all notifications are broadcast to all listeners. Subtree filtering (subscribing to `/sources/revenue/*`) is unimplemented.

---

## H. Architecture / Integration Gaps

| Gap | Detail |
|-----|--------|
| `ContextFocus` → Camera | Plan says `drillIn()` should "camera animates to frame it." `ContextEntry` has `bounds` but there is no wiring to `CameraPose`/`camera_framing`. |
| `IDocument` missing `getText` | `EditTextCommand` needs to read current text; the interface only has setters. |
| No mock `IDocument` | `edit_ops` tests can't be written without a document implementation to inject. |
| `PointerEvent::Type::Scroll` | Defined but unhandled everywhere. |
| `hit.hpp` interface mismatch | Takes `VisualId` but the plan says it should walk `render::Parent` from an ECS entity ID. Needs an entity accessor/callback. |

---

## I. Test Coverage Gaps

| Module | Test File | Coverage |
|--------|-----------|----------|
| `data_graph` | `data_graph_test.cpp` | Good |
| `command` | `command_test.cpp` | Good |
| `gestures` | `gestures_test.cpp` | Good |
| `interaction_state` | `interaction_state_test.cpp` | Partial (drag test has no assertions) |
| `focus` | `focus_test.cpp` | Good |
| `camera_framing` | `camera_framing_test.cpp` | Partial (no `orbitCamera` test) |
| `binding` | `binding_test.cpp` | Good |
| `hit` | — | **No test** |
| `edit_ops` | — | **No test** |
| `binding_resolver` | — | **No test** |
| `view_sync` | — | **No test** |

---

## J. CI / Tooling

- No `.github/workflows/` (no CI)
- No Doxygen config or API documentation generation
- No `.clang-format` or `.clang-tidy`
- No `CHANGELOG.md`, `CONTRIBUTING.md`, or `LICENSE`

---

## Priority Ranking

| Priority | Issue | Impact |
|----------|-------|--------|
| 1 | `hit.cpp` stub | Breaks all visual interaction — picking can't identify targets |
| 2 | `edit_ops.cpp` stubs (3 commands) | Data mutation undo/redo is non-functional |
| 3 | `EditTextCommand::oldText_` | Undo corrupts text |
| 4 | Transform system missing | All declarative data binding is pass-through only |
| 5 | `view_sync.cpp` stubs (4 methods) | Brushing/linking across views doesn't work |
| 6 | `orbitCamera()` no-op | Camera orbiting is broken |
| 7 | WASM bridge missing | Can't actually use the library from Web/WASM |
| 8 | Gesture recognizer gaps | LongPress, Brush, Scrub, Pinch, Pan never fire |
| 9 | InteractionState event gaps | 11 of 17 event types never produced |
| 10 | `DataGraph::subscribe` ignores path | No subtree filtering |
| 11 | Test coverage gaps | 4 modules untested |
| 12 | CI / tooling | No automation |
