# Extropian Interaction — Event-Driven Edit & Navigation Layer

> Shared interaction logic for the Composer ecosystem: events, commands, data binding, chart edits, zoom/context navigation.
> Dual‑platform (desktop + WebGL/WASM) — compile with Emscripten with zero changes.

## 1. Purpose

extropian-interaction answers:

> Given raw input + picking results + the document, what are the semantic edits, focus changes, and camera targets?

It does NOT answer:

- Where to render pixels (extropian-render)
- How to generate mesh data (extropian-ui)
- How to layout or style visuals (extropian-visualintent-renderer)
- How to capture raw input (app‑layer: SDL3 / DOM events)
- How to produce VisualIntent from AI (extropian-visualintent-conductor)

## 2. Position in the Architecture

```
Composer / Composer-Web (App Layer)
│  Raw input capture, SDL3 / DOM → PointerEvent, KeyboardEvent
│
└─ extropian-interaction (THIS REPO)  ← Dual‑platform (WASM)
   │  Events, hit resolution, gestures, interaction state machines
   │  Commands + undo/redo, data binding, chart/table edits
   │  Context focus (breadcrumb, drill in/out), camera framing
   │  View sync (brushing/linking, coordinated views)
   │  Pure C++. No GPU. No platform code. No I/O.
   │
   ├── extropian-render (PickerSystem, CameraSystem, GPU)
   ├── extropian-geometry (Bounds, MeshData types)
   └── extropian-core (math types, ECS)
```

**Key change from v1**: interaction logic was embedded in the composer app layer (`canvas_interaction_system`). It is now extracted into this shared library so desktop composer and composer-web use the same interaction semantics.

## 3. WASM Compatibility

extropian-interaction depends only on:
- `exd::core` (math types: Vec3f, Bounds, etc.)
- `exd::geometry` (Bounds alias, types)
- `exd::render` (Picker/selection/camera types)

All three dependencies compile cleanly to WASM. extropian-interaction has:
- No OpenGL headers
- No SDL3
- No platform‑specific code
- No threads or filesystem access
- No network calls

Compiles to WASM with Emscripten with zero changes.

## 4. Module Inventory

### 4.1 Events (`event.hpp`)

Migrated from composer's `interaction.hpp`, extended with chart/table‑specific events:

| Event | Purpose |
|---|---|
| `PointerEvent` | Move, Down, Up, Scroll, Enter, Leave |
| `KeyboardEvent` | Down, Up, Repeat with modifier tracking |
| `VisualHovered/Unhovered` | Hover enter/leave on a visual |
| `VisualFocused/Unfocused` | Focus gained/lost |
| `VisualSelected/Deselected` | Selection toggle (multi‑select with modifier) |
| `VisualActivated` | Double‑click / activate action |
| `VisualDragged` | Drag with start, current, delta |
| `VisualResized` | Resize with new bounds |
| `VisualTransformChanged` | General transform mutation |
| `ScrollChanged` | Scrollbar / zoom wheel |
| `TextEdited` | Inline text input result |
| `DataPointHovered` | Chart data point hover (series + point index) |
| `AxisBrushed` | Range selection on a chart axis |
| `SeriesReordered` | Drag‑reorder chart series |
| `ValueScrubbed` | Drag‑edit a data point value |
| `CellEdited` | Table cell edit result |
| `InteractionConfig` | Per‑visual interaction flags (hoverable, draggable, etc.) |

All events are in a `std::variant<VisualEvent>` for dispatch.

### 4.2 Hit Resolution (`hit.hpp`)

Resolves a render picker hit (entity ID) to a `VisualId` by walking the `render::Parent` hierarchy chain. Pure function — results depend only on input.

### 4.3 Gestures (`gestures.hpp`)

Stateful recognizer that consumes raw `PointerEvent` frames and emits gesture kinds:
- Tap, DoubleTap, LongPress
- Drag (with configurable threshold)
- Brush (range selection), Scrub (value drag)
- Pinch, Pan

Phases: Idle → Possible → Began → Changed → Ended / Cancelled.

### 4.4 Interaction State Machine (`interaction_state.hpp`)

Per‑visual hover, focus, selection, drag tracking. Driven by `PointerEvent` + `InteractionConfig`. Emits `VisualEvent` variants on state transitions. Supports multi‑select with modifier keys.

### 4.5 Commands & Undo/Redo (`command.hpp`)

Abstract `Command` base class with execute/undo/label/canMerge/merge. `CommandStack` provides bounded undo/redo history. Designed for app‑layer integration: commands mutate the document or data model, the stack manages history.

### 4.6 Edit Operations (`edit_ops.hpp`)

Concrete commands implementing common edit semantics. Operates through an `IDocument` interface (implemented by the app's `VisualDocument`) and `DataGraph`:

| Command | Purpose |
|---|---|
| `EditTextCommand` | Change text content of a visual (merge‑aware for continuous typing) |
| `PatchVisualCommand` | Move + resize a visual |
| `SetValueCommand` | Set a scalar in the DataGraph (merge‑aware for scrubbing) |
| `AppendDataPointCommand` | Add a data point to a chart series |
| `RemoveDataPointCommand` | Remove a data point from a chart series |
| `ReorderSeriesCommand` | Reorder chart series |

### 4.7 Data Graph (`data_graph.hpp`)

Registry of named data sources addressed by stable paths (e.g. `"/sources/revenue/quarterly"`). Sources hold:
- Scalar values (for settings/thresholds)
- 1D series (for bar/line/scatter data)
- 2D matrices (for heatmap/table data)

Mutation API: `setScalar`, `setSeries`, `appendToSeries`, `removeFromSeries`, `setMatrixValue`. Supports subscribers for change notification. Tracks dirty state for incremental resolution.

### 4.8 Data Binding (`binding.hpp`, `binding_resolver.hpp`)

Full declarative binding layer:
- `Binding` declares a descriptor field bound to a data source path with optional transform (Scale, Clamp, Normalize, ElementWise, Aggregate).
- `BindingRegistry` stores bindings per visual, resolves field values on source change.
- `BindingResolver` subscribes to the `DataGraph`, resolves affected bindings on source change, and notifies the app to mark visuals dirty for re‑compilation via extropian-ui.

Data flow: source mutation → `DataGraph::notify` → `BindingResolver` → `DirtyNotifier(visualId)` → `CanvasDirty` → incremental compile → `extropian-ui::generate_*_mesh()`.

### 4.9 Context Focus (`focus.hpp`)

Breadcrumb stack for drill‑in / drill‑out navigation through hierarchical visual contexts. Supports:
- `drillIn(context)` — push a new context, camera animates to frame it
- `drillOut()` — pop to parent
- `drillToLevel(n)` — navigate to a specific depth
- `ghostedAncestors()` — previous levels rendered at reduced opacity (persistent context)
- Root context is never popped — `isAtRoot()` guard

### 4.10 Camera Framing (`camera_framing.hpp`)

Pure math for camera pose computation:
- `frameBounds(bounds, aspectRatio, padding)` — compute a camera pose that frames given bounds
- `smoothFrame(current, target, t)` — interpolate between poses for animation
- `zoomToVisual(current, bounds)` — target pose for zooming to a specific visual
- `panCamera(pose, screenDelta, viewportW, viewportH)` — pan by screen‑space delta
- `orbitCamera(pose, deltaAzimuth, deltaElevation)` — orbit around target

The GPU camera *system* stays in extropian-render; this repo computes targets/poses.

### 4.11 View Sync (`view_sync.hpp`)

Manages brushing/linking across multiple coordinated views (renderer‑plan §10):
- `ViewConfig` per view: synchronization mode, shared selection/time/abstraction
- `setBrushRange(viewId, axisId, min, max)` — set brush selection on an axis
- Shared selection set propagated to synced views
- Shared time cursor (scrubbed position)
- View‑local filtering

## 5. Data Binding Design

### Binding Paths

A binding connects a **descriptor field path** to a **data source path**:

```cpp
Binding b;
b.fieldPath  = "chart.series[0].values";   // descriptor field
b.sourcePath = "/sources/revenue/quarterly"; // data graph path
b.index      = -1;                           // -1 = full series, >= 0 = element
b.transform  = { TransformKind::None, {} };  // optional transform
```

### Resolution

On source change:
1. `DataGraph::notify(path)` triggers listeners
2. `BindingResolver` queries `BindingRegistry::affectedVisuals(path)`
3. `BindingRegistry::resolveForSource(path)` evaluates bindings → `ResolvedField`s
4. `DirtyNotifier(visualId)` signals the app to re‑compile via extropian-ui

### Undo/Redo Interplay

Commands mutate sources via the `DataGraph` or document via `IDocument`. In both cases, the mutation triggers the binding resolvers, which marks visuals dirty. Undo reverses the mutation → resolvers re‑evaluate → visuals re‑compile. No special undo integration needed — it falls out naturally.

## 6. Integration with Composer

### Desktop Composer (extropian-canvas)

The composer's `canvas_interaction_system` becomes a thin adapter:

```cpp
// In canvas_interaction_system.cpp
void CanvasInteractionSystem::update(ecs::Registry& registry, double dt) {
    // 1. Get raw input from app
    auto& ptr = app_.currentPointerEvent();

    // 2. Delegate to extropian-interaction
    auto gesture = gestureRecognizer_.feed(ptr, dt);
    auto hit = resolve_hit(pickerSystem_.pick(ptr.position));
    auto event = interactionState_.process(ptr, gesture, hit.visualId, hit.config);

    // 3. Dispatch visual events
    if (event) dispatch(*event);

    // 4. Camera update
    cameraSystem_.setTarget(cameraPose_);
}
```

### Composer-Web (extropian-canvas-web)

extropian-interaction is compiled into the WASM binary. The bridge exposes:
- `interaction_feed_pointer(x, y, button, type)` → feeds raw pointer events
- `interaction_get_event()` → dequeues a `VisualEvent` (serialized as JSON)
- `interaction_execute_command(cmd_json)` → runs a command through the stack
- `interaction_bind(visualId, fieldPath, sourcePath)` → declares a binding
- `interaction_set_value(path, value)` → sets a DataGraph value

## 7. Dependencies

| Dependency | Purpose |
|---|---|
| `exd::core` | Math types (Vec3f, Bounds) |
| `exd::geometry` | Bounds type alias |
| `exd::render` | PickerSystem types, Parent component, hovered/selected components, CameraSystem types |

## 8. File Layout

```
include/exd/interaction/
├── interaction.hpp         # umbrella
├── event.hpp               # pointer/keyboard/visual/chart events + InteractionConfig
├── hit.hpp                 # picker hit → VisualId resolution
├── gestures.hpp            # tap/drag/brush/scrub gesture recognizer
├── interaction_state.hpp   # hover/focus/select/drag state machine
├── command.hpp             # Command + CommandStack (undo/redo)
├── edit_ops.hpp            # concrete edit commands + IDocument interface
├── data_graph.hpp          # data sources, paths, notification
├── binding.hpp             # declarative binding declarations + BindingRegistry
├── binding_resolver.hpp    # auto‑resolution on source change → dirty notifier
├── focus.hpp               # context breadcrumb stack, drill in/out
├── camera_framing.hpp      # bounds → camera pose math
└── view_sync.hpp           # brushing/linking, coordinated views

src/
├── event.cpp
├── hit.cpp
├── gestures.cpp
├── interaction_state.cpp
├── command.cpp
├── edit_ops.cpp
├── data_graph.cpp
├── binding.cpp
├── binding_resolver.cpp
├── focus.cpp
├── camera_framing.cpp
└── view_sync.cpp

tests/unit/
├── data_graph_test.cpp
├── command_test.cpp
├── gestures_test.cpp
├── interaction_state_test.cpp
├── focus_test.cpp
├── camera_framing_test.cpp
└── binding_test.cpp
```

## 9. Design Principles

- **Pure C++.** No GPU headers, no platform code, no I/O.
- **No state leakage.** State is explicit (InteractionState, GestureRecognizer, CommandStack) and resettable.
- **WASM‑safe.** Depends only on libraries that compile cleanly to WASM.
- **Inject dependencies.** `IDocument` and `DirtyNotifier` are interfaces; the app layer implements them.
- **Testable.** Every module has unit tests; gestures, commands, and focus are tested without a render context.
