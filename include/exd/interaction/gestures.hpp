#pragma once

#include <exd/interaction/event.hpp>

#include <cstdint>
#include <optional>

namespace exd::interaction
{

// ── Gesture phase ──

enum class GesturePhase : uint8_t
{
    Idle,
    Possible,    // touch/click started, gesture not yet determined
    Began,       // gesture confirmed
    Changed,     // gesture in progress
    Ended,       // normal completion
    Cancelled,   // aborted
};

// ── Gesture kind ──

enum class GestureKind : uint8_t
{
    None,
    Tap,
    DoubleTap,
    LongPress,
    Drag,
    Brush,        // range selection on axis
    Scrub,        // drag value slider
    Pinch,        // two-finger zoom (if available)
    Pan,
};

// ── Gesture recognizer ──

/// Stateful recognizer that consumes raw PointerEvents and emits Gesture events.
/// Call feed(pointerEvent, dt) each frame; query recognized gestures.
class GestureRecognizer
{
public:
    GestureRecognizer();
    ~GestureRecognizer();

    /// Feed a raw pointer event. Returns the recognized gesture kind (if any).
    std::optional<GestureKind> feed(const PointerEvent& event, double dt);

    /// Reset internal state (e.g. on view switch).
    void reset();

    /// Current gesture phase.
    GesturePhase phase() const { return phase_; }

    /// Position delta since gesture began (for drag/pan).
    math::Vec3f delta() const;

    /// Accumulated distance (for brush/scrub).
    double accumulatedDistance() const;

    /// Thresholds
    void setDragThreshold(float pixels)  { dragThreshold_ = pixels; }
    void setTapTimeoutMs(double ms)     { tapTimeoutMs_ = ms; }
    void setDoubleTapWindowMs(double ms) { doubleTapWindowMs_ = ms; }

private:
    GesturePhase phase_ = GesturePhase::Idle;
    GestureKind currentGesture_ = GestureKind::None;
    math::Vec3f startPosition_ = {};
    math::Vec3f lastPosition_ = {};
    double elapsedMs_ = 0.0;
    double totalTimeMs_ = 0.0;         // monotonic accumulator (not reset on gestures)
    float dragThreshold_ = 5.0f;
    double tapTimeoutMs_ = 250.0;
    double doubleTapWindowMs_ = 300.0;
    double lastTapTimeMs_ = -1000.0;   // time of last completed tap
    int tapCount_ = 0;
    bool moved_ = false;
};

}  // namespace exd::interaction
